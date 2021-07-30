//
// Created by khhan on 2021-06-21.
//

#ifndef KONAI3D_SHADER_PASS_H
#define KONAI3D_SHADER_PASS_H

#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/shader.h"
#include "src/engine/graphics/constant_buffer.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
enum class ShaderPassPriority {
    SHADER_PASS_PRIORITY_BACKGROUND,
    SHADER_PASS_PRIORITY_OPAQUE,
    SHADER_PASS_PRIORITY_ALPHACUT,
    SHADER_PASS_PRIORITY_TRANSPARENCY,
    SHADER_PASS_PRIORITY_OVERLAY
};

struct ShaderPassAttribute {
    bool enable_depth_test = true;
    bool enable_stencil_test = false;
    bool enable_blend_pass = false;
    bool front_counter_clockwise = false;

    ShaderPassPriority priority = ShaderPassPriority::SHADER_PASS_PRIORITY_OPAQUE;

    D3D12_BLEND_OP blend_op = D3D12_BLEND_OP_ADD;
    D3D12_BLEND_OP blend_alpha_op = D3D12_BLEND_OP_ADD;
    D3D12_BLEND src_blend = D3D12_BLEND_ONE;
    D3D12_BLEND dst_blend = D3D12_BLEND_ZERO;
    D3D12_BLEND src_alpha_blend = D3D12_BLEND_ONE;
    D3D12_BLEND dst_alpha_blend = D3D12_BLEND_ZERO;
    D3D12_CULL_MODE cull_mode = D3D12_CULL_MODE_BACK;
    D3D12_FILL_MODE fill_mode = D3D12_FILL_MODE_SOLID;

    UINT8 stencil_read_mask = D3D12_DEFAULT_STENCIL_READ_MASK;
    UINT8 stencil_write_mask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    D3D12_STENCIL_OP stencil_depth_fail_op_front = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP stencil_fail_op_front = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP stencil_pass_op_front = D3D12_STENCIL_OP_KEEP;
    D3D12_COMPARISON_FUNC stencil_func_front = D3D12_COMPARISON_FUNC_ALWAYS;

    D3D12_STENCIL_OP stencil_depth_fail_op_back = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP stencil_fail_op_back = D3D12_STENCIL_OP_KEEP;
    D3D12_STENCIL_OP stencil_pass_op_back = D3D12_STENCIL_OP_KEEP;
    D3D12_COMPARISON_FUNC stencil_func_back = D3D12_COMPARISON_FUNC_ALWAYS;

    D3D12_COMPARISON_FUNC depth_func = D3D12_COMPARISON_FUNC_LESS;
    D3D12_DEPTH_WRITE_MASK depth_write_mask = D3D12_DEPTH_WRITE_MASK_ALL;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

class ShaderPass
{
public:
    ShaderPass(std::shared_ptr<DeviceCom> device, DXGI_FORMAT depth_stencil_format, DXGI_FORMAT render_target_format);
    virtual ~ShaderPass() = default;

public:
    struct BindInfo {
        Shader::ShaderResourceType resource_type;
        UINT parameter_idx;
        std::string resource_name;
    };

public:
    bool Build(
            D3D12_INPUT_ELEMENT_DESC* inputElements,
            UINT numInputElements,
            ShaderPassAttribute* shaderPassAttr,
            std::shared_ptr<Shader> vs,
            std::shared_ptr<Shader> ps,
            std::shared_ptr<Shader> ds = nullptr,
            std::shared_ptr<Shader> hs = nullptr,
            std::shared_ptr<Shader> gs = nullptr
            );
    bool Clear();
    std::vector<CD3DX12_ROOT_PARAMETER> GetRootSignatureParamSlots();

    ShaderPassPriority GetPassPriority() const;
    ID3D12PipelineState* GetPipelineState() const;
    ID3D12RootSignature* GetRootSignature() const;


public:
    static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
    std::vector<BindInfo> GetStaticResrouceBindInfo(UINT currentFrameIdx);
    std::vector<BindInfo> GetMutableResourceBindInfo(UINT currentFrameIdx);

private:
    std::shared_ptr<DeviceCom> _device;
    std::unordered_map<std::string, UINT> _resources_idx;
    std::shared_ptr<Shader> _vs_shader;
    std::shared_ptr<Shader> _ps_shader;
    std::shared_ptr<Shader> _ds_shader;
    std::shared_ptr<Shader> _hs_shader;
    std::shared_ptr<Shader> _gs_shader;

    using ShaderResourceID = std::pair<Shader::ShaderResourceType, D3D12_SHADER_VISIBILITY>;
    std::unordered_map<std::string, ShaderResourceID> _static_resource;
    std::unordered_map<std::string, ShaderResourceID> _mutable_resource;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _root_signature;
    std::vector<CD3DX12_ROOT_PARAMETER> _params;

    ShaderPassPriority _priority;
    DXGI_FORMAT _depth_stencil_format;
    DXGI_FORMAT _render_target_format;
};
_END_ENGINE

#endif //KONAI3D_SHADER_PASS_H
