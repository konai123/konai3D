//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/shader_pass.h"
#include "src/engine/graphics/sampler.h"

_START_ENGINE
ShaderPass::ShaderPass(std::shared_ptr<DeviceCom> device, DXGI_FORMAT depth_stencil_format,
                       DXGI_FORMAT render_target_format)
:
_vs_shader(nullptr),
_ps_shader(nullptr),
_ds_shader(nullptr),
_hs_shader(nullptr),
_gs_shader(nullptr),
_device(device),
_depth_stencil_format(depth_stencil_format),
_render_target_format(render_target_format) {}

bool ShaderPass::Build(
        D3D12_INPUT_ELEMENT_DESC *inputElements,
        UINT numInputElements,
        ShaderPassAttribute *shaderPassAttr,
        std::shared_ptr<Shader> vs,
        std::shared_ptr<Shader> ps,
        std::shared_ptr<Shader> ds,
        std::shared_ptr<Shader> hs,
        std::shared_ptr<Shader> gs
) {
    Clear();
    _vs_shader = vs;
    _ps_shader = ps;
    _ds_shader = ds;
    _hs_shader = hs;
    _gs_shader = gs;
    std::vector<Shader *> shaders = {vs.get(), ps.get(), ds.get(), hs.get(), gs.get()};
    for (auto shader : shaders) {
        if (shader == nullptr) {
            if (vs == nullptr || ps == nullptr) {
                GRAPHICS_LOG_ERROR("Shader pass must contain PS, VS shaders");
                return false;
            }
            continue;
        };
        for (auto p : shader->_static_resource) {
            auto name = p.first;
            auto resourceType = p.second;
            /*
             * If per shader has same a resource type and a name, It will be merged one a parameter.
             * */
            if (_static_resource.contains(name)) {
                GRAPHICS_LOG_WARNING("Found a parameter with the same name in shader bundle : {}", name);
                return false;
            }

            _static_resource[name] = std::make_pair(resourceType, shader->_visivility[name]);
        }
        for (auto p : shader->_mutable_resource) {
            auto name = p.first;
            auto resourceType = p.second;
            /*
             * If per shader has same a resource type and a name, It will be merged one a parameter.
             * */
            if (_mutable_resource.contains(name)) {
                GRAPHICS_LOG_WARNING("Found a parameter with the same name in shader bundle : {}", name);
                return false;
            }

            _mutable_resource[name] = std::make_pair(resourceType, shader->_visivility[name]);
        }
    }

    /*
     * Building
     * */
    using m_resource = std::unordered_map<std::string, ShaderResourceID>;
    std::array<m_resource *, 2> resources = {&_static_resource, &_mutable_resource};

    std::array<int, 6> cbv_offset = {0, 0, 0, 0, 0, 0};
    std::array<int, 6> uav_offset = {0, 0, 0, 0, 0, 0};
    std::array<int, 6> srv_offset = {0, 0, 0, 0, 0, 0};
    auto static_sampler = GetStaticSamplers();
    D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    for (auto p_types : resources) {
        for (auto p : *p_types) {
            CD3DX12_ROOT_PARAMETER param;
            std::string name = p.first;
            auto type = p.second.first;
            auto vis = p.second.second;
            int d3d_visibility_en = vis;
            switch (type) {
                case Shader::ShaderResourceType::SHADER_RESOURCE_CB : {
                    int reg_number = cbv_offset[d3d_visibility_en];
                    if (d3d_visibility_en != D3D12_SHADER_VISIBILITY_ALL)
                        reg_number += cbv_offset[D3D12_SHADER_VISIBILITY_ALL];
                    param.InitAsConstantBufferView(reg_number, 0);
                    cbv_offset[d3d_visibility_en]++;
                    break;
                }
                case Shader::ShaderResourceType::SHADER_RESOURCE_SR : {
                    int reg_number = srv_offset[d3d_visibility_en];
                    if (d3d_visibility_en != D3D12_SHADER_VISIBILITY_ALL)
                        reg_number += srv_offset[D3D12_SHADER_VISIBILITY_ALL];
                    param.InitAsConstantBufferView(reg_number, 0);
                    srv_offset[d3d_visibility_en]++;
                    break;
                }

                case Shader::ShaderResourceType::SHADER_RESOURCE_UA : {
                    int reg_number = uav_offset[d3d_visibility_en];
                    if (d3d_visibility_en != D3D12_SHADER_VISIBILITY_ALL)
                        reg_number += uav_offset[D3D12_SHADER_VISIBILITY_ALL];
                    param.InitAsConstantBufferView(reg_number, 0);
                    uav_offset[d3d_visibility_en]++;
                    break;
                }
            }
            _resources_idx[name] = static_cast<UINT>(_params.size());
            _params.push_back(param);
        }
    }

    _root_signature = _device->CreateRootSignature(
            _params.data(), static_sampler.data(),
            static_cast<UINT>(static_sampler.size()), static_cast<UINT>(_params.size()), flags
    );

    if (_root_signature == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create root signature.");
        return false;
    }

    /*
     * Build PSO
     * */
    ShaderPassAttribute pass_attr;
    if (shaderPassAttr != nullptr) {
        pass_attr = *shaderPassAttr;
    }
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {0};
    pso_desc.PrimitiveTopologyType = pass_attr.topology_type;

    pso_desc.DepthStencilState.FrontFace.StencilDepthFailOp = pass_attr.stencil_depth_fail_op_front;
    pso_desc.DepthStencilState.FrontFace.StencilFailOp = pass_attr.stencil_fail_op_front;
    pso_desc.DepthStencilState.FrontFace.StencilPassOp = pass_attr.stencil_pass_op_front;
    pso_desc.DepthStencilState.FrontFace.StencilFunc = pass_attr.stencil_func_front;

    pso_desc.DepthStencilState.BackFace.StencilDepthFailOp = pass_attr.stencil_depth_fail_op_back;
    pso_desc.DepthStencilState.BackFace.StencilFailOp = pass_attr.stencil_fail_op_back;
    pso_desc.DepthStencilState.BackFace.StencilPassOp = pass_attr.stencil_pass_op_back;
    pso_desc.DepthStencilState.BackFace.StencilFunc = pass_attr.stencil_func_back;

    pso_desc.DepthStencilState.DepthFunc = pass_attr.depth_func;
    pso_desc.DepthStencilState.DepthWriteMask = pass_attr.depth_write_mask;
    pso_desc.DepthStencilState.DepthEnable = pass_attr.enable_depth_test;
    pso_desc.DepthStencilState.StencilEnable = pass_attr.enable_stencil_test;
    pso_desc.DepthStencilState.StencilReadMask = pass_attr.stencil_read_mask;
    pso_desc.DepthStencilState.StencilWriteMask = pass_attr.stencil_write_mask;

    pso_desc.BlendState.AlphaToCoverageEnable = false;
    pso_desc.BlendState.IndependentBlendEnable = false;

    pso_desc.SampleDesc.Count = 1;
    pso_desc.SampleDesc.Quality = 0;

    pso_desc.DSVFormat = _depth_stencil_format;
    pso_desc.RTVFormats[0] = _render_target_format;

    pso_desc.NumRenderTargets = 1;
    D3D12_RENDER_TARGET_BLEND_DESC &b_desc = pso_desc.BlendState.RenderTarget[0];
    b_desc.BlendEnable = pass_attr.enable_blend_pass;
    b_desc.BlendOp = pass_attr.blend_op;
    b_desc.BlendOpAlpha = pass_attr.blend_alpha_op;
    b_desc.DestBlend = pass_attr.dst_blend;
    b_desc.DestBlendAlpha = pass_attr.dst_alpha_blend;
    b_desc.SrcBlend = pass_attr.src_blend;
    b_desc.SrcBlendAlpha = pass_attr.src_alpha_blend;

    b_desc.LogicOpEnable = false;
    b_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_RASTERIZER_DESC r_desc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    r_desc.CullMode = pass_attr.cull_mode;
    r_desc.FillMode = pass_attr.fill_mode;
    r_desc.FrontCounterClockwise = pass_attr.front_counter_clockwise;

    pso_desc.RasterizerState = r_desc;

    if (_vs_shader != nullptr) {
        pso_desc.VS = {reinterpret_cast<BYTE *>(_vs_shader->_shader_byte_code->GetBufferPointer()),
                       _vs_shader->_shader_byte_code->GetBufferSize()};
    }

    if (_ps_shader != nullptr) {
        pso_desc.PS = {reinterpret_cast<BYTE *>(_ps_shader->_shader_byte_code->GetBufferPointer()),
                       _ps_shader->_shader_byte_code->GetBufferSize()};
    }

    if (_ds_shader != nullptr) {
        pso_desc.DS = {reinterpret_cast<BYTE *>(_ds_shader->_shader_byte_code->GetBufferPointer()),
                       _ds_shader->_shader_byte_code->GetBufferSize()};
    }

    if (_gs_shader != nullptr) {
        pso_desc.GS = {reinterpret_cast<BYTE *>(_gs_shader->_shader_byte_code->GetBufferPointer()),
                       _gs_shader->_shader_byte_code->GetBufferSize()};
    }

    if (_hs_shader != nullptr) {
        pso_desc.HS = {reinterpret_cast<BYTE *>(_hs_shader->_shader_byte_code->GetBufferPointer()),
                       _hs_shader->_shader_byte_code->GetBufferSize()};
    }

    pso_desc.pRootSignature = _root_signature.Get();
    pso_desc.InputLayout = {inputElements, numInputElements};
    pso_desc.SampleMask = UINT_MAX;

    _pso = _device->CreatePipelineState(&pso_desc);
    if (_pso == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create pso.");
        return false;
    }
    _priority = pass_attr.priority;
    return true;
}

std::vector<CD3DX12_ROOT_PARAMETER> ShaderPass::GetRootSignatureParamSlots() {
    return _params;
}

ShaderPassPriority ShaderPass::GetPassPriority() const {
    return _priority;
}

ID3D12PipelineState *ShaderPass::GetPipelineState() const {
    return _pso.Get();
}

ID3D12RootSignature *ShaderPass::GetRootSignature() const {
    return _root_signature.Get();
}

bool ShaderPass::Clear() {
    _vs_shader = nullptr;
    _ps_shader = nullptr;
    _ds_shader = nullptr;
    _hs_shader = nullptr;
    _gs_shader = nullptr;
    _static_resource.clear();
    _mutable_resource.clear();
    _params.clear();

    _pso = nullptr;
    _root_signature = nullptr;
    _priority = ShaderPassPriority::SHADER_PASS_PRIORITY_OPAQUE;
    return true;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> ShaderPass::GetStaticSamplers() {
    // Applications usually only need a handful of samplers.  So just define them all up front
    // and keep them available as part of the root signature.

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


std::vector<ShaderPass::BindInfo> ShaderPass::GetStaticResrouceBindInfo(UINT currentFrameIdx) {
    std::vector<ShaderPass::BindInfo> bind_info;
    for (auto &p : _static_resource) {
        std::string name = p.first;
        Shader::ShaderResourceType type = p.second.first;

        ShaderPass::BindInfo info;
        info.resource_name = name;
        info.resource_type = type;
        info.parameter_idx = _resources_idx[name];
        bind_info.push_back(info);
    }
    return bind_info;
}

std::vector<ShaderPass::BindInfo> ShaderPass::GetMutableResourceBindInfo(UINT currentFrameIdx) {
    std::vector<ShaderPass::BindInfo> bind_info;
    for (auto &p : _mutable_resource) {
        std::string name = p.first;
        Shader::ShaderResourceType type = p.second.first;

        ShaderPass::BindInfo info;
        info.resource_name = name;
        info.resource_type = type;
        info.parameter_idx = _resources_idx[name];
        bind_info.push_back(info);
    }
    return bind_info;
}

_END_ENGINE