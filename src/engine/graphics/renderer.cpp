//
// Created by khhan on 2021-06-14.
//

#include "src/engine/graphics/renderer.h"
#include "src/engine/graphics/render_screen.h"

_START_ENGINE
Renderer::Renderer()
:
_command_queue(nullptr),
_command_list(nullptr),
_dsv_buffer_full_frame(nullptr),
_render_target_width(0),
_render_target_height(0),
_full_frame_viewport({0}),
_full_frame_scissor_rect({0}),
_current_frame(0),
_num_pre_frames(1) {
    _rendering_options = {.v_sync=true, .scale_factor = 1.0f};
}

bool Renderer::Initiate(HWND hWnd, UINT width, UINT height, UINT renderWidth, UINT renderHeight, UINT numCPUPreRender,
                        std::shared_ptr<UIRenderer> uiRenderer) {
    _num_pre_frames = numCPUPreRender;
    _render_target_width = width;
    _render_target_height = height;

    _device = std::make_shared<DeviceCom>();
    _resource_heap = std::make_shared<ResourceDescriptorHeap>();

    if (!_device->Initiate(hWnd, width, height, _num_backbuffer, _backbuffer_format)) {
        GRAPHICS_LOG_ERROR("Failed to initialize direct12.");
        return false;
    }

    _command_queue = _device->GetCommandQueue();
    if (_command_queue == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create a CommandQueue.");
        return false;
    }

    _command_list = _device->CreateGraphicsCommandList();
    if (_command_list == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create a CommandList.");
        return false;
    }

    if (!_frame_buffer_pool.Initiate(_device.get(), _command_queue, numCPUPreRender, D3D12_COMMAND_LIST_TYPE_DIRECT)) {
        GRAPHICS_LOG_ERROR("Failed to initialize CommandExecutor.");
        return false;
    };

    if (!_resource_heap->Initiate(_device.get())) {
        GRAPHICS_LOG_ERROR("Failed to initialize ResourceDescriptorHeap.");
        return false;
    }

    if (!CreateFullFrameRenderTargetBufferAndView(false))
        return false;

    if (!CreateDepthStencilBufferAndView(false, &_full_frame_depth_stencil_view, _dsv_buffer_full_frame,
                                         _render_target_width, _render_target_height))
        return false;

    _full_frame_viewport = {0, 0, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    _full_frame_scissor_rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};

    _gui_view = _resource_heap->GetShaderResourceHeapDescriptor();

    if (uiRenderer != nullptr) {
        _ui_renderer = uiRenderer;
        if (!_ui_renderer->Initiate(
                hWnd, _device.get(), _backbuffer_format, _resource_heap->GetShaderResourceHeap().Get(), &_gui_view,
                width, height, numCPUPreRender)
                ) {
            GRAPHICS_LOG_ERROR("Failed to initialize imgui renderer.");
            return false;
        }
    }

    return true;
}

void Renderer::OnRender(
        float delta,
        RenderScreen **renderScreens,
        UINT numRenderScreen,
        ResourceMap<DrawInfo*>* meshMap,
        ResourceMap<ShaderPass*>* shaderPassMap
        ) {
    auto frame_buffer = _frame_buffer_pool.RequestFrameBuffer(_device.get());
    frame_buffer->_allocator->Reset();
    _command_list->Reset(frame_buffer->_allocator.Get(), NULL);

    ID3D12DescriptorHeap *heaps[] = {
            _resource_heap->GetShaderResourceHeap().Get(),
            _resource_heap->GetSamplerHeap().Get()
    };
    _command_list->SetDescriptorHeaps(_countof(heaps), heaps);
    {
        /*
         * Rendering RenderLists
         * */
        for (UINT i = 0; i < numRenderScreen; i++) {
            RenderScreen *render_screen = renderScreens[i];
            if (render_screen->DeletedQueueSize() > 0) {
                WaitAllFrame();
                render_screen->CollectGarbage();
            }

            auto render_target = render_screen->GetRenderTargetResource();
            auto render_target_viwe = render_screen->GetRenderTargetHeapDesc();
            auto dsv_view = render_screen->GetDepthStencilHeapDesc();

            _command_list->RSSetViewports(1, render_screen->GetViewPort());
            _command_list->RSSetScissorRects(1, render_screen->GetScissorRect());
            D3D12_RESOURCE_BARRIER barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
                    render_target,
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_RENDER_TARGET
            );
            _command_list->ResourceBarrier(1, &barrier_enter);

            _command_list->OMSetRenderTargets(1, &render_target_viwe->_cpu_handle,
                                              true, &dsv_view->_cpu_handle);

            _command_list->ClearRenderTargetView(render_target_viwe->_cpu_handle, _clear_color, 0, nullptr);
            _command_list->ClearDepthStencilView(dsv_view->_cpu_handle,
                                                 D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                                 1.0f, 0, 0, nullptr);

            auto renderObjs = render_screen->GetRenderObjects();
            DrawRenderObject(_command_list.Get(), ShaderPassPriority::SHADER_PASS_PRIORITY_BACKGROUND, renderObjs,
                             render_screen, meshMap, shaderPassMap);
            DrawRenderObject(_command_list.Get(), ShaderPassPriority::SHADER_PASS_PRIORITY_OPAQUE, renderObjs,
                             render_screen, meshMap, shaderPassMap);
            DrawRenderObject(_command_list.Get(), ShaderPassPriority::SHADER_PASS_PRIORITY_ALPHACUT, renderObjs,
                             render_screen, meshMap, shaderPassMap);
            DrawZOrderedRenderObject(_command_list.Get(), ShaderPassPriority::SHADER_PASS_PRIORITY_TRANSPARENCY,
                                     renderObjs, render_screen->GetCameraPosition(), render_screen, meshMap, shaderPassMap);
            DrawRenderObject(_command_list.Get(), ShaderPassPriority::SHADER_PASS_PRIORITY_OVERLAY, renderObjs,
                             render_screen, meshMap, shaderPassMap);

            barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
                    render_target,
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    D3D12_RESOURCE_STATE_COMMON
            );
            _command_list->ResourceBarrier(1, &barrier_enter);
        }
    }

    _command_list->RSSetViewports(1, &_full_frame_viewport);
    _command_list->RSSetScissorRects(1, &_full_frame_scissor_rect);

    D3D12_RESOURCE_BARRIER barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
            _device->GetCurrentBackBuffer().Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    _command_list->ResourceBarrier(1, &barrier_enter);

    UINT backbuffer_idx = _device->GetCurrentBackBufferIndex();
    _command_list->OMSetRenderTargets(1, &_backbuffer_view[backbuffer_idx]._cpu_handle, true,
                                      &_full_frame_depth_stencil_view._cpu_handle);

    _command_list->ClearRenderTargetView(_backbuffer_view[backbuffer_idx]._cpu_handle, _clear_color, 0, nullptr);
    _command_list->ClearDepthStencilView(_full_frame_depth_stencil_view._cpu_handle,
                                         D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                         1.0f, 0, 0, nullptr);


    /*
     * IMGUI Render pass
     * */
    {
        _ui_renderer->OnGUIRender(delta, _command_list.Get());
    }

    D3D12_RESOURCE_BARRIER barrier_out = CD3DX12_RESOURCE_BARRIER::Transition(
            _device->GetCurrentBackBuffer().Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
    );
    _command_list->ResourceBarrier(1, &barrier_out);

    _command_list->Close();

    ID3D12CommandList *cmd_list[] = {_command_list.Get()};
    _command_queue->ExecuteCommandLists(_countof(cmd_list), cmd_list);
    _device->Present(_rendering_options.v_sync);
    _frame_buffer_pool.DiscardFrameBuffer(frame_buffer);
    _current_frame = (_current_frame + 1) % (_num_pre_frames);
}

void Renderer::DrawRenderObject(
        ID3D12GraphicsCommandList *cmdList,
        ShaderPassPriority priority,
        std::vector<RenderObject *> renderObjects,
        RenderScreen *renderScreen,
        ResourceMap<DrawInfo*>* meshMap,
        ResourceMap<ShaderPass*>* shaderPassMap
) {
    std::unordered_map<ShaderPass *, std::vector<RenderObject *>> sorted_objs;
    std::vector<ShaderPass *> shader_passes;
    for (UINT i = 0; i < renderObjects.size(); i++) {
        auto pass_name = renderObjects[i]->GetShaderPassName();
        if (!shaderPassMap->Contains(pass_name)) {
            /*Change default shader pass*/
            auto name = shaderPassMap->GetDefaultResourceName();
            EngineAssert(shaderPassMap->Contains(name));
            renderObjects[i]->SetShaderPassName(name);
            pass_name = name;
        }

        auto pass = shaderPassMap->GetResource(pass_name);
        EngineAssert(pass != nullptr);
        if (pass->GetPassPriority() != priority) continue;
        sorted_objs[pass].push_back(renderObjects[i]);
    }

    for (auto &p : sorted_objs) {
        ShaderPass *pass = p.first;
        EngineAssert(pass != nullptr);
        auto objects = sorted_objs[pass];

        cmdList->SetPipelineState(pass->GetPipelineState());
        cmdList->SetGraphicsRootSignature(pass->GetRootSignature());
        auto static_binds = pass->GetStaticResrouceBindInfo(_current_frame);
        for (auto &info : static_binds) {
            BindResource(cmdList, info, renderScreen);
        }

        for (auto &obj : objects) {
            RenderObject *render_obj = obj;
            EngineAssert(render_obj != nullptr);

            auto draw_info_id = render_obj->GetDrawInfoID();
            if (!meshMap->Contains(draw_info_id)) {
                auto name = meshMap->GetDefaultResourceName();
                EngineAssert(meshMap->Contains(name));
                obj->SetDrawInfoID(name);
                draw_info_id = name;
            }

            auto draw_info = meshMap->GetResource(draw_info_id);
            VertexBuffer *vbBuffer = draw_info->_vertex_buffer.get();

            D3D_PRIMITIVE_TOPOLOGY topo_type = draw_info->_type;
            D3D12_VERTEX_BUFFER_VIEW vb_view = vbBuffer->VertexBufferView();
            D3D12_INDEX_BUFFER_VIEW ib_view = vbBuffer->IndexBufferView();

            auto mutable_binds = pass->GetMutableResourceBindInfo(_current_frame);
            for (auto &info : mutable_binds) {
                BindResource(cmdList, info, render_obj);
            }

            cmdList->IASetPrimitiveTopology(topo_type);
            cmdList->IASetIndexBuffer(&ib_view);
            cmdList->IASetVertexBuffers(0, 1, &vb_view);
            cmdList->DrawIndexedInstanced(
                    draw_info->_index_count, 1, draw_info->_start_index_location,
                    draw_info->_base_vertex_location, 0);
        }
    }
}

void Renderer::DrawZOrderedRenderObject(ID3D12GraphicsCommandList *cmdList,
                                        ShaderPassPriority priority,
                                        std::vector<RenderObject *> renderObjects,
                                        DirectX::XMFLOAT3 cameraPosition,
                                        RenderScreen *renderScreen,
                                        ResourceMap<DrawInfo*>* meshMap,
                                        ResourceMap<ShaderPass*>* shaderPassMap
                                        ) {
    std::priority_queue<std::pair<float, RenderObject *>> z_oreder_q;

    for (UINT i = 0; i < renderObjects.size(); ++i) {
        auto obj = renderObjects[i];
        auto pass_name = obj->GetShaderPassName();
        if (!shaderPassMap->Contains(pass_name)) {
            /*Change default shader pass*/
            auto name = shaderPassMap->GetDefaultResourceName();
            EngineAssert(shaderPassMap->Contains(name));
            renderObjects[i]->SetShaderPassName(name);
            pass_name = name;
        }

        auto pass = shaderPassMap->GetResource(pass_name);

        if (pass->GetPassPriority() != priority) continue;
        EngineAssert(obj != nullptr);

        auto x = obj->GetWorldMatrix().r[3].m128_f32[0];
        auto y = obj->GetWorldMatrix().r[3].m128_f32[1];
        auto z = obj->GetWorldMatrix().r[3].m128_f32[2];

        DirectX::XMVECTOR obj_position = DirectX::XMVectorSet(x, y, z, 1.0f);
        DirectX::XMVECTOR camera_position = DirectX::XMLoadFloat3(&cameraPosition);
        DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(obj_position, camera_position);
        float length = DirectX::XMVector3Length(dir).m128_f32[0];
        z_oreder_q.push(std::make_pair(-length, obj));
    }

    ShaderPass *last_pass = nullptr;
    bool is_first = true;
    while (!z_oreder_q.empty()) {
        RenderObject *render_obj = z_oreder_q.top().second;
        z_oreder_q.pop();
        auto pass_name = render_obj->GetShaderPassName();
        auto pass = shaderPassMap->GetResource(pass_name);
        if (is_first || last_pass != pass) {
            is_first = false;
            last_pass = pass;

            cmdList->SetPipelineState(pass->GetPipelineState());
            cmdList->SetGraphicsRootSignature(pass->GetRootSignature());
            auto static_binds = pass->GetStaticResrouceBindInfo(_current_frame);
            for (auto &info : static_binds) {
                BindResource(cmdList, info, renderScreen);
            }
        }

        auto mutable_binds = pass->GetMutableResourceBindInfo(_current_frame);
        for (auto &info : mutable_binds) {
            BindResource(cmdList, info, render_obj);
        }

        auto draw_info_id = render_obj->GetDrawInfoID();
        if (!meshMap->Contains(draw_info_id)) {
            auto name = meshMap->GetDefaultResourceName();
            EngineAssert(meshMap->Contains(name));
            render_obj->SetDrawInfoID(name);
            draw_info_id = name;
        }

        auto draw_info = meshMap->GetResource(draw_info_id);
        VertexBuffer *vbBuffer = draw_info->_vertex_buffer.get();

        D3D_PRIMITIVE_TOPOLOGY topo_type = draw_info->_type;
        D3D12_VERTEX_BUFFER_VIEW vb_view = vbBuffer->VertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ib_view = vbBuffer->IndexBufferView();

        cmdList->IASetPrimitiveTopology(topo_type);
        cmdList->IASetIndexBuffer(&ib_view);
        cmdList->IASetVertexBuffers(0, 1, &vb_view);
        cmdList->DrawIndexedInstanced(
                draw_info->_index_count, 1, draw_info->_start_index_location,
                draw_info->_base_vertex_location, 0);
    }
}

void Renderer::BindResource(ID3D12GraphicsCommandList *cmdList, ShaderPass::BindInfo bindInfo, Rendered *rendered) {
    if (bindInfo.resource_type == Shader::SHADER_RESOURCE_CB) {
        auto constant_buffer = rendered->GetConstantBuffer(bindInfo.resource_name);
        EngineAssert(constant_buffer != nullptr);
        auto resource = constant_buffer->GetResource(_current_frame);
        EngineAssert(resource != nullptr);
        cmdList->SetGraphicsRootConstantBufferView(bindInfo.parameter_idx, resource->GetGPUVirtualAddress());
    } else if (bindInfo.resource_type == Shader::SHADER_RESOURCE_SR) {
        //TODO
//        CoreAssert(resource != nullptr);
//        cmdList->SetGraphicsRootShaderResourceView(bindInfo.parameter_idx, resource->GetGPUVirtualAddress());
    } else if (bindInfo.resource_type == Shader::SHADER_RESOURCE_UA) {
        //TODO
//        CoreAssert(resource != nullptr);
//        cmdList->SetGraphicsRootUnorderedAccessView(bindInfo.parameter_idx, resource->GetGPUVirtualAddress());
    }
}

void Renderer::OnResizeFullFrame(UINT width, UINT height) {
    _render_target_width = width;
    _render_target_height = height;
    _full_frame_viewport.Width = static_cast<float>(width);
    _full_frame_viewport.Height = static_cast<float>(height);
    _full_frame_scissor_rect = {0, 0, static_cast<int>(width), static_cast<int>(height)};

    WaitAllFrame();
    _device->ResizeSwapchain(_render_target_width, _render_target_height);
    CreateDepthStencilBufferAndView(true, &_full_frame_depth_stencil_view, _dsv_buffer_full_frame, _render_target_width,
                                    _render_target_height);
    CreateFullFrameRenderTargetBufferAndView(true);

    _ui_renderer->OnResize(width, height);
}

void Renderer::OnDestroy() {
    WaitAllFrame();
    _ui_renderer->OnDestroy();
    _device->Close();
}

bool Renderer::CreateDepthStencilBufferAndView(bool isRecreation, HeapDescriptor *heapDescriptor,
                                               Microsoft::WRL::ComPtr<ID3D12Resource> &resource, UINT width,
                                               UINT height) {
//Create Depth Stencil Buffer
    CD3DX12_RESOURCE_DESC depth_stencil_desc;
    depth_stencil_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_stencil_desc.Alignment = 0;
    depth_stencil_desc.Width = width;
    depth_stencil_desc.Height = height;
    depth_stencil_desc.DepthOrArraySize = 1;
    depth_stencil_desc.MipLevels = 1;
    depth_stencil_desc.Format = _depth_stencil_format;
    depth_stencil_desc.SampleDesc.Count = 1;
    depth_stencil_desc.SampleDesc.Quality = 0;
    depth_stencil_desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depth_stencil_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    CD3DX12_CLEAR_VALUE clear_value;
    clear_value.Format = _depth_stencil_format;
    clear_value.DepthStencil.Depth = 1.0f;
    clear_value.DepthStencil.Stencil = 0;

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    resource = nullptr;
    resource = _device->CreateResource(
            &properties,
            &depth_stencil_desc,
            &clear_value,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
    );

    if (!isRecreation)
        *heapDescriptor = _resource_heap->GetDepthStencilHeapDescriptor();
    if (!heapDescriptor->IsVaild()) {
        CORE_LOG_ERROR("Failed to create depth stencil view.");
        return false;
    }
    _device->CreatDescriptorHeapView<D3D12_DEPTH_STENCIL_VIEW_DESC>(
            resource.Get(), nullptr, heapDescriptor->_cpu_handle, nullptr
    );

    return true;
}

bool Renderer::CreateFullFrameRenderTargetBufferAndView(bool isRecreation) {
    //Save Backbuffer from swapchains
    for (int i = 0; i < _num_backbuffer; i++) {
        auto backbuffer = _device->GetBackBuffer(i);

        if (!isRecreation) {
            auto heap_desc = _resource_heap->GetRenderTargetHeapDescriptor();
            _backbuffer_view[i] = heap_desc;
        }

        if (!_backbuffer_view[i].IsVaild()) {
            CORE_LOG_ERROR("Failed to create rendertarget view.");
            return false;
        }
        _device->CreatDescriptorHeapView<D3D12_RENDER_TARGET_VIEW_DESC>
                (backbuffer.Get(), nullptr, _backbuffer_view[i]._cpu_handle, nullptr);
    }

    return true;
}

void Renderer::SetRenderingOptions(RenderingOptions options) {
    _rendering_options = options;
}


void Renderer::WaitAllFrame() {
    _frame_buffer_pool.WaitAllExecute();
}

UINT Renderer::GetCurrentFrameIndex() {
    return _current_frame;
}

std::unique_ptr<RenderScreen> Renderer::InstanceRenderScreen(UINT width, UINT height) {
    auto render_screen = std::make_unique<RenderScreen>(_device, _resource_heap, width, height, _backbuffer_format,
                                                        _depth_stencil_format, _clear_color);
    return std::move(render_screen);
}

std::unique_ptr<ShaderPass> Renderer::InstanceShaderPass() {
    return std::make_unique<ShaderPass>(_device, _depth_stencil_format, _backbuffer_format);
}

std::unique_ptr<ConstantBuffer> Renderer::InstanceConstanceBuffer() {
    return std::make_unique<ConstantBuffer>(_device, _num_pre_frames);
}

std::unique_ptr<Sampler> Renderer::InstanceSampler() {
    return std::make_unique<Sampler>(_device, _resource_heap);
}

std::unique_ptr<VertexBuffer> Renderer::InstanceVertexBuffer(void *vertex, UINT numVertex,
                                                             UINT vertexSize, UINT32 *indices, UINT numIndex,
                                                             bool isDynamic) {
    auto frame_buffer = _frame_buffer_pool.RequestFrameBuffer(_device.get());
    frame_buffer->_allocator->Reset();
    _command_list->Reset(frame_buffer->_allocator.Get(), NULL);
    auto vb = std::make_unique<VertexBuffer>(_device.get(), _command_list.Get(), vertex, numVertex,
                                             vertexSize, indices, numIndex, isDynamic);
    ID3D12CommandList *cmd_list[] = {_command_list.Get()};

    _command_list->Close();
    _command_queue->ExecuteCommandLists(_countof(cmd_list), cmd_list);
    _frame_buffer_pool.DiscardFrameBuffer(frame_buffer);
    _current_frame = (_current_frame + 1) % (_num_pre_frames);
    return std::move(vb);
}

std::vector<D3D12_INPUT_ELEMENT_DESC> Renderer::InstanceInputElements(InputElement *elements, UINT numElements) {
    std::vector<D3D12_INPUT_ELEMENT_DESC> descs;
    for (UINT i = 0; i < numElements; i++) {
        D3D12_INPUT_ELEMENT_DESC desc = {
                elements[i].semantic_name,
                elements[i].semantic_index,
                elements[i].format,
                0,
                elements[i].aligned_byte_offset,
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                0
        };
        descs.push_back(desc);
    }
    return descs;
}

_END_ENGINE
