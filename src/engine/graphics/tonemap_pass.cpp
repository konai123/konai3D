//
// Created by korona on 2021-09-20.
//

#include "src/engine/graphics/renderer.h"
#include "src/engine/graphics/shader.h"
#include "src/engine/graphics/tonemap_pass.h"

_START_ENGINE

namespace {
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 1> StaticSamplers() {
    const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
            0, // shaderRegister
            D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW
    return {pointClamp};
}
}

ToneMapper::ToneMapper(std::shared_ptr<DeviceCom> deviceCom)
:
_device_com(deviceCom)
{}

bool ToneMapper::Initiate() {
    if (!BuildRootSignature()) {
        GRAPHICS_LOG_ERROR("Failed to initialize root signature");
        return false;
    }
    if (!BuildPSO()) {
        GRAPHICS_LOG_ERROR("Failed to initialize pipeline state");
        return false;
    }
    return true;
}

void ToneMapper::Render(float delta, ID3D12GraphicsCommandList6 *command_list, RenderScreen* screen, HeapDescriptorHandle* shaderResourceView) {
    auto render_target = screen->GetRenderTargetHeapDesc();
    command_list->SetGraphicsRootSignature(_root_signature.Get());
    command_list->SetPipelineState(_pso.Get());
    command_list->SetGraphicsRootDescriptorTable(0, shaderResourceView->GpuHandle);
    command_list->ClearRenderTargetView(render_target->CpuHandle, Renderer::ClearColor, 0, nullptr);
    command_list->OMSetRenderTargets(1, &render_target->CpuHandle, true, nullptr);

    auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, screen->Width, screen->Height);
    auto scissor_rect= CD3DX12_RECT(0, 0, screen->Width, screen->Height);
    command_list->RSSetViewports(1, &viewport);
    command_list->RSSetScissorRects(1, &scissor_rect);
    command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    command_list->DrawInstanced(3, 1, 0, 0);
}

bool ToneMapper::BuildRootSignature() {
    std::vector<CD3DX12_DESCRIPTOR_RANGE1> range;
    CD3DX12_ROOT_PARAMETER1 srv_param;
    {
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE |
                                             D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;

        CD3DX12_DESCRIPTOR_RANGE1 g_texture;
        g_texture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,  flags, 0);

        range.push_back(g_texture);
        srv_param.InitAsDescriptorTable(static_cast<UINT>(range.size()), range.data());
    }

    std::vector<CD3DX12_ROOT_PARAMETER1> params = {
            srv_param
    };

    auto static_samplers = StaticSamplers();
    _root_signature = _device_com->CreateRootSignature(params.data(), static_samplers.data(), static_cast<UINT>(static_samplers.size()),
                                                   static_cast<UINT>(params.size()));

    if (_root_signature == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create root signature.");
        return false;
    }
    return true;
}

bool ToneMapper::BuildPSO() {
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {0};
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(CD3DX12_DEFAULT());
    pso_desc.DepthStencilState.DepthEnable = false;

    pso_desc.BlendState = CD3DX12_BLEND_DESC(CD3DX12_DEFAULT());
    pso_desc.SampleDesc.Count = 1;
    pso_desc.SampleDesc.Quality = 0;
    pso_desc.RTVFormats[0] = Renderer::BackbufferFormat;
    pso_desc.DSVFormat = Renderer::DepthStencilFormat;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(CD3DX12_DEFAULT());
    pso_desc.pRootSignature = _root_signature.Get();
    pso_desc.SampleMask = UINT_MAX;

    pso_desc.VS = {reinterpret_cast<BYTE *>(Shader::ToneMapPass.VS->GetBufferPointer()),
                   Shader::ToneMapPass.VS->GetBufferSize()};

    pso_desc.PS = {Shader::ToneMapPass.PS->GetBufferPointer(),
                   Shader::ToneMapPass.PS->GetBufferSize()};

    _pso = _device_com->CreatePipelineState(&pso_desc);
    if (_pso== nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create pso.");
        return false;
    }
    return true;
}


_END_ENGINE
