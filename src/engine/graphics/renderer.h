//
// Created by khhan on 2021-06-14.
//

#ifndef KONAI3D_RENDERER_H
#define KONAI3D_RENDERER_H

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/frame_buffer_pool.h"
#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/ui_renderer.h"
#include "src/engine/graphics/render_object.h"
#include "src/engine/graphics/shader_pass.h"
#include "src/engine/graphics/render_screen.h"
#include "src/engine/graphics/rendered.h"
#include "src/engine/graphics/sampler.h"
#include "src/engine/graphics/macros.h"
#include "src/engine/graphics/resource_map.h"

_START_ENGINE
struct RenderingOptions {
    bool v_sync;
    float scale_factor;
};

struct InputElement {
public:
    InputElement(LPCSTR name, UINT semanticIndex, DXGI_FORMAT format, UINT alignedByteOffset)
    :
    semantic_name(name),
    semantic_index(semanticIndex),
    format(format),
    aligned_byte_offset(alignedByteOffset) {}

    LPCSTR semantic_name;
    UINT semantic_index;
    DXGI_FORMAT format;
    UINT aligned_byte_offset;
};

class Renderer {
public:
    Renderer();
    ~Renderer() = default;
    Renderer &operator=(const Renderer &) = delete;
    Renderer &operator=(Renderer &&) noexcept = default;
    Renderer(const Renderer &) = delete;
    Renderer(Renderer &&) noexcept = default;

public:
    bool Initiate(HWND hWnd, UINT width, UINT height, UINT renderWidth, UINT renderHeight, UINT numCPUPreRender,
                  std::shared_ptr<UIRenderer> uiRenderer = nullptr);
    void OnRender(
            float delta,
            RenderScreen **renderScreens,
            UINT numRenderScreen,
            ResourceMap<DrawInfo*>* meshMap,
            ResourceMap<ShaderPass*>* shaderPassMap
    );
    void OnResizeFullFrame(UINT width, UINT height);
    void OnDestroy();
    void SetRenderingOptions(RenderingOptions options);
    void WaitAllFrame();
    UINT GetCurrentFrameIndex();
    std::unique_ptr<RenderScreen> InstanceRenderScreen(UINT width, UINT height);
    std::unique_ptr<ShaderPass> InstanceShaderPass();
    std::unique_ptr<ConstantBuffer> InstanceConstanceBuffer();
    std::unique_ptr<Sampler> InstanceSampler();
    std::unique_ptr<VertexBuffer> InstanceVertexBuffer(void *vertex, UINT numVertex, UINT vertexSize,
                                                       UINT32 *indices, UINT numIndex, bool isDynamic);
    std::vector<D3D12_INPUT_ELEMENT_DESC> InstanceInputElements(InputElement *elements, UINT numElements);

private:
    bool CreateDepthStencilBufferAndView(bool isRecreation, HeapDescriptor *heapDescriptor,
                                         Microsoft::WRL::ComPtr<ID3D12Resource> &resource, UINT width, UINT height);
    bool CreateFullFrameRenderTargetBufferAndView(bool isRecreation);
    void DrawRenderObject(ID3D12GraphicsCommandList *cmdList,
                          ShaderPassPriority priority,
                          std::vector<RenderObject *> renderObjects,
                          RenderScreen *renderScreen,
                          ResourceMap<DrawInfo*>* meshMap,
                          ResourceMap<ShaderPass*>* shaderPassMap
                          );
    void DrawZOrderedRenderObject(ID3D12GraphicsCommandList *cmdList,
                                  ShaderPassPriority priority,
                                  std::vector<RenderObject *> renderObjects,
                                  DirectX::XMFLOAT3 cameraPosition,
                                  RenderScreen *renderScreen,
                                  ResourceMap<DrawInfo*>* meshMap,
                                  ResourceMap<ShaderPass*>* shaderPassMap
                                  );
    void BindResource(ID3D12GraphicsCommandList *cmdList, ShaderPass::BindInfo bindInfo, Rendered *rendered);

private:
    static const size_t _num_backbuffer = 3;
    static const DXGI_FORMAT _backbuffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT _depth_stencil_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

private:
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> _command_queue;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _command_list;
    Microsoft::WRL::ComPtr<ID3D12Resource> _dsv_buffer_full_frame;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _upload_command_list;

    HeapDescriptor _gui_view;
    HeapDescriptor _full_frame_depth_stencil_view;
    HeapDescriptor _backbuffer_view[_num_backbuffer];

private:
    std::shared_ptr<DeviceCom> _device;
    std::shared_ptr<ResourceDescriptorHeap> _resource_heap;

    UINT _render_target_width;
    UINT _render_target_height;
    D3D12_VIEWPORT _full_frame_viewport;
    D3D12_RECT _full_frame_scissor_rect;
    FrameBufferPool _frame_buffer_pool;
    RenderingOptions _rendering_options;

    std::shared_ptr<UIRenderer> _ui_renderer;
    inline static DirectX::XMVECTORF32 _clear_color;
    UINT _current_frame;
    UINT _num_pre_frames;
};

_END_ENGINE

#endif //KONAI3D_RENDERER_H
