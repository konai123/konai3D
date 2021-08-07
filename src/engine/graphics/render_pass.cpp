//
// Created by korona on 2021-08-02.
//

#include "src/engine/graphics/render_pass.h"

#include "src/engine/graphics/renderer.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/shader.h"
#include "src/engine/graphics/upload_buffer.h"

_START_ENGINE

namespace {
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> StaticSamplers() {
    const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
            0, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            1, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
            2, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
            3, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
            4, // shaderRegister
            D3D12_FILTER_ANISOTROPIC, // filter
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
            0.0f,                             // mipLODBias
            8);                               // maxAnisotropy

    const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
            5, // shaderRegister
            D3D12_FILTER_ANISOTROPIC, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
            0.0f,                              // mipLODBias
            8);                                // maxAnisotropy

    return {
            pointWrap, pointClamp,
            linearWrap, linearClamp,
            anisotropicWrap, anisotropicClamp};
}
}

RenderPass::RenderPass(std::shared_ptr<DeviceCom> deviceCom)
:
_device(deviceCom) {}

bool RenderPass::Initiate() {
    if (!BuildRootSignature()) {
        GRAPHICS_LOG_ERROR("Failed to initialize root signature");
        return false;
    }
    if (!BuildPipelineState()) {
        GRAPHICS_LOG_ERROR("Failed to initialize pipeline state");
        return false;
    }

    BuildResourceBuffer();

    return true;
}

void RenderPass::Render(
        float delta,
        std::vector<RenderScreen *> screens,
        ID3D12GraphicsCommandList *command_list,
        UINT currentFrameIndex,
        MeshMap *meshMap,
        TextureMap *textureMap,
        MaterialMap *materialMap,
        ResourceDescriptorHeap *heaps
) {
    {
        command_list->SetGraphicsRootSignature(_root_signature.Get());
        command_list->SetPipelineState(_opaque_pso.Get());

        /*Bind TextureTables*/
        command_list->SetGraphicsRootShaderResourceView(4,
                                                        _rw_buffer_material->GetResource(
                                                                currentFrameIndex)->GetGPUVirtualAddress());
        command_list->SetGraphicsRootDescriptorTable(5,
                                                     heaps->GetShaderResourceHeap()->GetGPUDescriptorHandleForHeapStart());

        for (UINT i = 0; i < screens.size(); i++) {
            RenderScreen *render_screen = screens[i];

            auto render_target = render_screen->GetRenderTargetResource();
            auto render_target_viwe = render_screen->GetRenderTargetHeapDesc();
            auto dsv_view = render_screen->GetDepthStencilHeapDesc();

            command_list->RSSetViewports(1, render_screen->GetViewPort());
            command_list->RSSetScissorRects(1, render_screen->GetScissorRect());
            D3D12_RESOURCE_BARRIER barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
                    render_target,
                    D3D12_RESOURCE_STATE_COMMON,
                    D3D12_RESOURCE_STATE_RENDER_TARGET
            );
            command_list->ResourceBarrier(1, &barrier_enter);

            command_list->OMSetRenderTargets(1, &render_target_viwe->CpuHandle,
                                             true, &dsv_view->CpuHandle);

            command_list->ClearRenderTargetView(render_target_viwe->CpuHandle, Renderer::ClearColor, 0, nullptr);
            command_list->ClearDepthStencilView(dsv_view->CpuHandle,
                                                D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
                                                1.0f, 0, 0, nullptr);

            {
                CBPerFrame per_frame{
                        .view_mat =  render_screen->ViewMatrix,
                        .inverse_view_mat = render_screen->InverseViewMatrix,
                        .projection_mat = render_screen->ProjectionMatrix,
                };
                _cb_buffer_per_frames->UpdateData(&per_frame, currentFrameIndex);
            }

            command_list->SetGraphicsRootConstantBufferView(1, _cb_buffer_per_frames->
                    GetResource(currentFrameIndex)->GetGPUVirtualAddress());

            auto names = render_screen->GetRenderObjectList();
            int obj_count = 0;
            for (auto& name : names) {
                auto obj = render_screen->GetRenderObject(name);

                if (obj_count >= MAX_RENDER_OBJECT) {
                    GRAPHICS_LOG_WARNING("Maximum render object overed!");
                    break;
                }
                {
                    {
                        //Validation
                        int material_id = materialMap->GetMaterialID(obj->MaterialName);
                        if (material_id == -1) {
                            GRAPHICS_LOG_ERROR("Failed to render, Invalid material:{}", obj->MaterialName);
                            continue;
                        }

                        auto material_desc = materialMap->GetMaterialDesc(obj->MaterialName);
                        auto resource = textureMap->GetResource(material_desc.value().DiffuseTexturePath);
                        if (!resource) {
                            GRAPHICS_LOG_ERROR("Failed to render, Invalid texture:{}",
                                               material_desc.value().DiffuseTexturePath);
                            continue;
                        }

                        ShaderType::Material mat {
                            .DiffuseTextureIndex = resource->Handle._heap_index,
                        };
                        _rw_buffer_material->UpdateData(&mat, material_id, currentFrameIndex);

                        CBPerObject per_object{
                                .world_mat = obj->WorldMatrix,
                                .material_id = material_id,
                        };
                        _cb_buffer_per_objects[obj_count]->UpdateData(&per_object, currentFrameIndex);
                    }
                }
                command_list->SetGraphicsRootConstantBufferView(0, _cb_buffer_per_objects[obj_count]->
                        GetResource(currentFrameIndex)->GetGPUVirtualAddress());
                if (!meshMap->Contains(obj->MeshID)) {
                    GRAPHICS_LOG_ERROR("Cannot find mesh id : {}", obj->MeshID);
                    continue;
                }
                auto mesh_resource = meshMap->GetResource(obj->MeshID);

                command_list->SetGraphicsRootShaderResourceView(2,
                                                                mesh_resource->_vertex_buffer->GetGPUVirtualAddress());
                command_list->SetGraphicsRootShaderResourceView(3,
                                                                mesh_resource->_index_buffer->GetGPUVirtualAddress());
                command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                command_list->DrawInstanced(mesh_resource->_ib_byte_size / sizeof(UINT), 1, 0, 0);
                obj_count++;
            }
            barrier_enter = CD3DX12_RESOURCE_BARRIER::Transition(
                    render_target,
                    D3D12_RESOURCE_STATE_RENDER_TARGET,
                    D3D12_RESOURCE_STATE_COMMON
            );
            command_list->ResourceBarrier(1, &barrier_enter);
        }
    }
}

bool RenderPass::BuildRootSignature() {
    CD3DX12_ROOT_PARAMETER1 cb_per_object_param;
    {
        cb_per_object_param.InitAsConstantBufferView(0, 0);
    }

    CD3DX12_ROOT_PARAMETER1 cb_per_frame_param;
    {
        cb_per_frame_param.InitAsConstantBufferView(1, 0);
    }

    CD3DX12_ROOT_PARAMETER1 vertex_param;
    {
        vertex_param.InitAsShaderResourceView(1, 0);
    }

    CD3DX12_ROOT_PARAMETER1 index_param;
    {
        index_param.InitAsShaderResourceView(1, 1);
    }

    CD3DX12_ROOT_PARAMETER1 material_param;
    {
        material_param.InitAsShaderResourceView(1, 2);
    }

    std::vector<CD3DX12_DESCRIPTOR_RANGE1> range;
    CD3DX12_ROOT_PARAMETER1 srv_param;
    {
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE |
                D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;

        /*Texture2D gTexture2DTable[] : register(t0, space100)*/
        CD3DX12_DESCRIPTOR_RANGE1 g_texture;
        g_texture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT_MAX, 0, 100,  flags, 0);
        /*Texture2DArray gTexture2DArrayTable[] : register(t0, space101)*/
        CD3DX12_DESCRIPTOR_RANGE1 g_texture_arr;
        g_texture_arr.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT_MAX, 0, 101, flags, 0);

        range.push_back(g_texture);
        range.push_back(g_texture_arr);
        srv_param.InitAsDescriptorTable(static_cast<UINT>(range.size()), range.data());
    }

    std::vector<CD3DX12_ROOT_PARAMETER1> params = {
            cb_per_object_param,
            cb_per_frame_param,
            vertex_param,
            index_param,
            material_param,
            srv_param
    };

    auto static_samplers = StaticSamplers();
    _root_signature = _device->CreateRootSignature(params.data(), static_samplers.data(), static_cast<UINT>(static_samplers.size()),
                                                   static_cast<UINT>(params.size()));

    if (_root_signature == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create root signature.");
        return false;
    }
    return true;
}

bool RenderPass::BuildPipelineState() {
    {
        /*OPAQUE PSO*/
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {0};
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
        pso_desc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
        pso_desc.SampleDesc.Count = 1;
        pso_desc.SampleDesc.Quality = 0;
        pso_desc.RTVFormats[0] = Renderer::BackbufferFormat;
        pso_desc.DSVFormat = Renderer::DepthStencilFormat;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
        pso_desc.pRootSignature = _root_signature.Get();
        pso_desc.SampleMask = UINT_MAX;

        pso_desc.VS = {reinterpret_cast<BYTE *>(Shader::RenderPass.VS->GetBufferPointer()),
                       Shader::RenderPass.VS->GetBufferSize()};

        pso_desc.PS = {Shader::RenderPass.PS->GetBufferPointer(),
                       Shader::RenderPass.PS->GetBufferSize()};

        _opaque_pso = _device->CreatePipelineState(&pso_desc);
        if (_opaque_pso == nullptr) {
            GRAPHICS_LOG_ERROR("Failed to create pso.");
            return false;
        }
    }
    return true;
}

bool RenderPass::BuildResourceBuffer() {
    CBPerObject null_obj;
    CBPerFrame null_frame;
    ShaderType::Material material;

    for (int i = 0; i < MAX_RENDER_OBJECT; i++)
    {
        _cb_buffer_per_objects[i] = std::move(std::make_unique<ConstantBuffer>(_device, Renderer::NumPreFrames));
        _cb_buffer_per_objects[i]->SetData(&null_obj, sizeof(CBPerObject));
    }

    _cb_buffer_per_frames = std::move(std::make_unique<ConstantBuffer>(_device, Renderer::NumPreFrames));
    _cb_buffer_per_frames->SetData(&null_frame, sizeof(CBPerObject));

    _rw_buffer_material = std::move(std::make_unique<RWResourceBuffer>(_device, Renderer::NumPreFrames));

    _rw_buffer_material->SetData(&material, MaterialMap::MaxMaterial,
                                 sizeof(ShaderType::Material));

    return false;
}

_END_ENGINE
