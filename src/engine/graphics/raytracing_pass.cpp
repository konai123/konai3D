//
// Created by korona on 2021-08-23.
//

#include "src/engine/graphics/raytracing_pass.h"

#include "src/engine/graphics/blas.h"
#include "src/engine/graphics/shader.h"
#include "src/engine/graphics/renderer.h"
#include "src/engine/graphics/shader_types.h"

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

Raytracer::Raytracer(std::shared_ptr<DeviceCom> deviceCom):
_rtpso(nullptr),
_device(deviceCom),
_total_frame_cnt(0),
_integration_cnt(0)
{
}

bool Raytracer::Initiate() {
    _tlas.resize(Renderer::NumPreFrames);
    for (UINT i = 0; i < _tlas.size(); i++) {
        _tlas[i].Initialize(RenderScreen::MAX_RENDER_OBJECT, _device.get());
    }

    if(!BuildGlobalRootSignature()) {
        GRAPHICS_LOG_ERROR("Failed To Create Global Rootsignature");
        return false;
    }
    if (!BuildHitgroupRootSignature()) {
        GRAPHICS_LOG_ERROR("Failed To Create Hit Group Signature");
        return false;
    }
    if(!BuildRSPipelineState()) {
        GRAPHICS_LOG_ERROR("Failed To Create Raytracing State Pipeline");
        return false;
    }
    if (!BuildResourceBuffer()) {
        GRAPHICS_LOG_ERROR("Failed To Create ResourceBuffers");
        return false;
    }


    return true;
}

void Raytracer::Render(
        float delta,
        RenderScreen * screen,
        ID3D12GraphicsCommandList6 *command_list,
        UINT currentFrameIndex,
        MeshMap *meshMap,
        TextureMap *textureMap,
        MaterialMap *materialMap,
        ResourceDescriptorHeap *heaps
) {
    {
        RenderScreen *render_screen = screen;
        auto render_target = render_screen->GetRenderTargetResource();

        //Update Lights
        auto lights = render_screen->GetLightList();
        for (int i = 0; i < lights.size(); i++) {
            auto light_ptr = render_screen->GetLight(lights[i]);
            ShaderType::Light l = {
                    .LightType = light_ptr->LightType,
                    .Position = light_ptr->Position,
                    .Radius = light_ptr->Radius
            };
            _rw_buffer_light->UpdateData(&l, i, currentFrameIndex);
        }

        {
            auto camera_info = render_screen->GetCameraInfo();
            ShaderType::Camera camera = {
                    .Position = camera_info.CameraPosition,
                    .Pad0 = 0.0f,
                    .Direction = camera_info.CameraDirection,
                    .Pad1 = 0.0f,
                    .UpVector = camera_info.CameraUp,
                    .Pad2 = 0.0f,
                    .AspectRatio = camera_info.AspectRatio,
                    .Fov = camera_info.Fov,
                    .Near = camera_info.Near,
                    .Far = camera_info.Far
            };

            int env_tex_idx = -1;
            if (render_screen->EnvTextureKey.has_value()) {
                auto tex_opt = textureMap->GetResource(render_screen->EnvTextureKey.value());
                if (tex_opt.has_value()) {
                    env_tex_idx = tex_opt->Handle._heap_index;
                }
            }

            CBPerFrame per_frame{
                .Camera = camera,
                .RenderTargetIdx = static_cast<UINT>(render_screen->GetShaderResourceHeapDesc()->_heap_index),
                .TotalFrameCount = _total_frame_cnt,
                .IntegrationCount = _integration_cnt,
                .NumberOfLight = static_cast<UINT>(lights.size()),
                .EnvTextureIdx = env_tex_idx
            };
            _cb_buffer_per_frames->UpdateData(&per_frame, currentFrameIndex);
        }

        {
            //Update Materials
            auto materials = materialMap->GetMaterialList();
            for (auto &mat: materials) {
                auto material_desc = materialMap->GetMaterialDesc(mat);
                auto material_id = materialMap->GetMaterialID(mat);
                if (material_id == -1) {
                    continue;
                }

                materialMap->UpdateMaterial(mat, material_desc.value());

                auto resource = textureMap->GetResource(material_desc.value().BaseColorTexturePath);
                if (!resource) {
                    continue;
                }

                ShaderType::Material material{
                        .BaseColorTextureIndex = resource->Handle._heap_index,
                        .MaterialType = material_desc->MaterialType,
                        .Fuzz = material_desc->Fuzz,
                        .RefractIndex = material_desc->RefractIndex,
                        .EmittedColor = material_desc->EmittedColor,
                        .Pad0 = 0.0f,
                        .Albedo = material_desc->Albedo,
                        .Pad1 = 0.0f,
                        .UseBaseColorTexture = material_desc->UseBaseColorTexture ? 1 : 0
                };
                _rw_buffer_material->UpdateData(&material, material_id, currentFrameIndex);
            }
        }

        _tlas[currentFrameIndex].Clear();
        std::vector<RenderObject*> objs;
        auto names = render_screen->GetRenderObjectList();
        int obj_count = 0;
        for (auto& name : names) {
            auto obj = render_screen->GetRenderObject(name);
            if (obj_count >= RenderScreen::MAX_RENDER_OBJECT) {
                GRAPHICS_LOG_WARNING("Maximum render object overed!");
                break;
            }


            auto mat_name = obj->MaterialName;
            auto mat_desc = materialMap->GetMaterialDesc(mat_name);

            if (!mat_desc.has_value()) continue;
            if (!meshMap->Contains(obj->MeshID)) continue;
            if (!textureMap->Contains(mat_desc->BaseColorTexturePath)) continue;

            auto mesh_resources = meshMap->GetResources(obj->MeshID);
            auto blas = mesh_resources->Meshes[obj->SubmeshID]->Blas.get();

            D3D12_RAYTRACING_INSTANCE_DESC inst_desc;
            inst_desc.InstanceID = obj->ObjectID;
            inst_desc.InstanceMask = 0xFF;
            inst_desc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
            inst_desc.AccelerationStructure = blas->ResultDataBuffer->GetGPUVirtualAddress();
            inst_desc.InstanceContributionToHitGroupIndex = obj_count * blas->GetSize();
            DirectX::XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(inst_desc.Transform), obj->WorldMatrix);

            GRAPHICS_LOG_DEBUG("FRAME IDX: {}", currentFrameIndex);
            _tlas[currentFrameIndex].AddInstance(inst_desc);
            objs.push_back(obj);
            obj_count++;
        }

        if (true)
        {
            if (!_tlas[currentFrameIndex].Generate(_device.get(), command_list)) {
            }

            if (!BuildRSShaderTable(command_list)) {
            }

            if (!UpdateHitgroupTable(meshMap, materialMap, objs, currentFrameIndex, command_list)) {
            }

            if (screen->Updated) {
                Reset();
                screen->Updated = false;
            }

            command_list->SetComputeRootSignature(_global_root_signature.Get());
            command_list->SetPipelineState1(_rtpso.Get());

            /*Bind TextureTables*/
            command_list->SetComputeRootShaderResourceView(2,
                                                           _rw_buffer_material->GetResource(
                                                                   currentFrameIndex)->GetGPUVirtualAddress());

            command_list->SetComputeRootShaderResourceView(5,
                                                           _rw_buffer_light->GetResource(
                                                                   currentFrameIndex)->GetGPUVirtualAddress());

            command_list->SetComputeRootDescriptorTable(3,
                                                        heaps->GetShaderResourceHeap()->GetGPUDescriptorHandleForHeapStart());

            command_list->SetComputeRootDescriptorTable(4,
                                                        heaps->GetShaderResourceHeap()->GetGPUDescriptorHandleForHeapStart());

            command_list->SetComputeRootConstantBufferView(0, _cb_buffer_per_frames->
                    GetResource(currentFrameIndex)->GetGPUVirtualAddress());


            command_list->SetComputeRootShaderResourceView(1, _tlas[currentFrameIndex].ResultDataBuffer->GetGPUVirtualAddress());

            _ray_gen_shader_table_resource->ResourceBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, currentFrameIndex, command_list);
            _miss_shader_table_resource->ResourceBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, currentFrameIndex, command_list);
            if (_hit_group_shader_table_resource != nullptr)
                _hit_group_shader_table_resource->ResourceBarrier(D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, currentFrameIndex, command_list);

            D3D12_DISPATCH_RAYS_DESC desc = {};

            desc.RayGenerationShaderRecord.StartAddress = _ray_gen_shader_table_resource->GetResource(currentFrameIndex)->GetGPUVirtualAddress();
            desc.RayGenerationShaderRecord.SizeInBytes = _ray_gen_shader_table_resource->GetResourceBytesSize();

            desc.MissShaderTable.StartAddress = _miss_shader_table_resource->GetResource(currentFrameIndex)->GetGPUVirtualAddress();
            desc.MissShaderTable.SizeInBytes = _miss_shader_table_resource->GetResourceBytesSize();
            desc.MissShaderTable.StrideInBytes = _miss_table.MaxRecordSize;

            if (_hit_group_shader_table_resource) {
                desc.HitGroupTable.StartAddress = _hit_group_shader_table_resource->GetResource(currentFrameIndex)->GetGPUVirtualAddress();
                desc.HitGroupTable.SizeInBytes = _hit_group_shader_table_resource->GetResourceBytesSize();
                desc.HitGroupTable.StrideInBytes = _hitgroup_table.MaxRecordSize;
            }

            desc.Width = render_screen->Width;
            desc.Height = render_screen->Height;
            desc.Depth = 1;

            command_list->DispatchRays(&desc);

            _ray_gen_shader_table_resource->ResourceBarrier(D3D12_RESOURCE_STATE_COMMON, currentFrameIndex, command_list);
            _miss_shader_table_resource->ResourceBarrier(D3D12_RESOURCE_STATE_COMMON, currentFrameIndex, command_list);
            if (_hit_group_shader_table_resource != nullptr)
                _hit_group_shader_table_resource->ResourceBarrier(D3D12_RESOURCE_STATE_COMMON, currentFrameIndex, command_list);

            _integration_cnt++;
            _total_frame_cnt++;
        }
    }
}

bool Raytracer::BuildGlobalRootSignature() {
    CD3DX12_ROOT_PARAMETER1 cb_per_frame_param;
    {
        cb_per_frame_param.InitAsConstantBufferView(1, 0);
    }

    CD3DX12_ROOT_PARAMETER1 material_param;
    {
        material_param.InitAsShaderResourceView(1, 0);
    }

    CD3DX12_ROOT_PARAMETER1 raytracing_acceleration_structure;
    {
        raytracing_acceleration_structure.InitAsShaderResourceView(1, 1);
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

    std::vector<CD3DX12_DESCRIPTOR_RANGE1> range_uav;
    CD3DX12_ROOT_PARAMETER1 uav_param;
    {
        D3D12_DESCRIPTOR_RANGE_FLAGS flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE |
                                             D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE;

        CD3DX12_DESCRIPTOR_RANGE1 g_rw_texture;
        g_rw_texture.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, UINT_MAX, 0, 100, flags, 0);

        range_uav.push_back(g_rw_texture);
        uav_param.InitAsDescriptorTable(static_cast<UINT>(range_uav.size()), range_uav.data());
    }

    CD3DX12_ROOT_PARAMETER1 light_param;
    {
        light_param.InitAsShaderResourceView(1, 2);
    }

    std::vector<CD3DX12_ROOT_PARAMETER1> params = {
            cb_per_frame_param,
            raytracing_acceleration_structure,
            material_param,
            srv_param,
            uav_param,
            light_param
    };

    auto static_samplers = StaticSamplers();
    _global_root_signature = _device->CreateRootSignature(params.data(), static_samplers.data(), static_cast<UINT>(static_samplers.size()),
                                                   static_cast<UINT>(params.size()));

    if (_global_root_signature == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create root signature.");
        return false;
    }
    return true;
}

bool Raytracer::BuildHitgroupRootSignature() {
    CD3DX12_ROOT_PARAMETER1 cb_per_object_param;
    {
        cb_per_object_param.InitAsConstants(1, 0, 0);
    }

    CD3DX12_ROOT_PARAMETER1 vertex_buffer;
    {
        vertex_buffer.InitAsShaderResourceView(2, 0);
    }

    CD3DX12_ROOT_PARAMETER1 index_buffer;
    {
        index_buffer.InitAsShaderResourceView(2, 1);
    }

    std::vector<CD3DX12_ROOT_PARAMETER1> params = {
            cb_per_object_param,
            vertex_buffer,
            index_buffer
    };

    _hit_group_root_signature = _device->CreateRootSignature(params.data(), nullptr, 0,
                                                          static_cast<UINT>(params.size()), D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);

    if (_hit_group_root_signature == nullptr) {
        GRAPHICS_LOG_ERROR("Failed to create root signature.");
        return false;
    }
    return true;
}

bool Raytracer::UpdateHitgroupTable(MeshMap* meshMap, MaterialMap* matMap, std::vector<RenderObject*> objs, UINT currentFrame,
                                    ID3D12GraphicsCommandList* cmdList) {
    _hitgroup_table.Clear();

    for (int i = 0; i < objs.size(); i++) {
        ShaderRecord hit_group;
        auto shader_id = _rtpso_info->GetShaderIdentifier(L"HitGroup");
        hit_group.AddShaderID(shader_id);

        auto mat_name = objs[i]->MaterialName;
        int matID = matMap->GetMaterialID(mat_name);
        if (matID == -1) {
            continue;
        }
        hit_group.AddField<UINT, sizeof(UINT)>(matID);

        //Padding
        hit_group.AddField<UINT, sizeof(UINT)>(0);

        auto meshID = objs[i]->MeshID;
        if (!meshMap->Contains(meshID)) {
            continue;
        }
        auto submesh = meshMap->GetResources(meshID)->Meshes[objs[i]->SubmeshID].get();
        hit_group.AddField<D3D12_GPU_VIRTUAL_ADDRESS, sizeof(D3D12_GPU_VIRTUAL_ADDRESS)>(
                submesh->VertexBuffer->GetGPUVirtualAddress() + submesh->BaseVertexLocation * sizeof(Vertex)
        );
        hit_group.AddField<D3D12_GPU_VIRTUAL_ADDRESS, sizeof(D3D12_GPU_VIRTUAL_ADDRESS)>(
                submesh->IndexBuffer->GetGPUVirtualAddress() + submesh->StartIndexLocation * sizeof(UINT)
        );
        _hitgroup_table.AddRecord(hit_group);
    }

    if (_hitgroup_table.Records.empty()) {
        return false;
    }

    if (_hit_group_shader_table_resource == nullptr
    || _hitgroup_table.GetBytesSize() > _hit_group_shader_table_resource->GetResourceBytesSize()) {
        if (_hit_group_shader_table_resource != nullptr) {
            _hit_group_shader_table_resource->SafeRelease();
        }

        _hit_group_shader_table_resource = std::make_unique<FrameResourceBuffer>(_device, Renderer::NumPreFrames);
        _hit_group_shader_table_resource->Initialize(CD3DX12_RESOURCE_DESC::Buffer(_hitgroup_table.GetBytesSize()));
    }

    if (!_hitgroup_table.Generate(_hit_group_shader_table_resource.get(), currentFrame, cmdList)) {
        return false;
    }
    return true;
}

bool Raytracer::BuildRSShaderTable(ID3D12GraphicsCommandList* cmdList) {

    if (_ray_gen_shader_table_resource == nullptr) {
        ShaderRecord ray_gen;
        _ray_gen_table.Clear();

        ray_gen.AddShaderID(_rtpso_info->GetShaderIdentifier(L"RayGen_12"));
        _ray_gen_table.AddRecord(ray_gen);

        _ray_gen_shader_table_resource = std::make_unique<FrameResourceBuffer>(_device, Renderer::NumPreFrames);
        _ray_gen_shader_table_resource->Initialize(CD3DX12_RESOURCE_DESC::Buffer(_ray_gen_table.GetBytesSize()));

        for (UINT i = 0; i < Renderer::NumPreFrames; i++) {
            if (!_ray_gen_table.Generate(_ray_gen_shader_table_resource.get(), i, cmdList)) {
                return false;
            }
        }
    }

    if (_miss_shader_table_resource == nullptr) {
        ShaderRecord miss, shadow_miss;

        _miss_table.Clear();
        miss.AddShaderID(_rtpso_info->GetShaderIdentifier(L"Miss_5"));
        _miss_table.AddRecord(miss);

        shadow_miss.AddShaderID(_rtpso_info->GetShaderIdentifier(L"Miss_Shadow"));
        _miss_table.AddRecord(shadow_miss);

        _miss_shader_table_resource = std::make_unique<FrameResourceBuffer>(_device, Renderer::NumPreFrames);
        _miss_shader_table_resource->Initialize(CD3DX12_RESOURCE_DESC::Buffer(_miss_table.GetBytesSize()));

        for (UINT i = 0; i < Renderer::NumPreFrames; i++) {
            if (!_miss_table.Generate(_miss_shader_table_resource.get(), i, cmdList)) {
                return false;
            }
        }
    }

    return true;
}

bool Raytracer::BuildRSPipelineState() {
    UINT idx = 0;
    std::vector<D3D12_STATE_SUBOBJECT> sub_objects(10);

    D3D12_EXPORT_DESC export_desc_ray_gen = {};
    export_desc_ray_gen.Name = L"RayGen_12";
    export_desc_ray_gen.ExportToRename = L"RayGen";
    export_desc_ray_gen.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_DXIL_LIBRARY_DESC lib_desc_ray_gen;
    lib_desc_ray_gen.DXILLibrary.BytecodeLength = Shader::RaytracePass.Raygen->GetBufferSize();
    lib_desc_ray_gen.DXILLibrary.pShaderBytecode = Shader::RaytracePass.Raygen->GetBufferPointer();
    lib_desc_ray_gen.NumExports = 1;
    lib_desc_ray_gen.pExports = &export_desc_ray_gen;

    D3D12_STATE_SUBOBJECT desc = {};
    desc.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    desc.pDesc = &lib_desc_ray_gen;

    sub_objects[idx++] = desc;

    std::vector<D3D12_EXPORT_DESC> misses;
    D3D12_EXPORT_DESC export_desc_miss = {};
    export_desc_miss.Name = L"Miss_5";
    export_desc_miss.ExportToRename = L"Miss";
    export_desc_miss.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_EXPORT_DESC export_desc_miss_shadow = {};
    export_desc_miss_shadow.Name = L"Miss_Shadow";
    export_desc_miss_shadow.ExportToRename = L"ShadowMiss";
    export_desc_miss_shadow.Flags = D3D12_EXPORT_FLAG_NONE;

    misses.push_back(export_desc_miss);
    misses.push_back(export_desc_miss_shadow);

    D3D12_DXIL_LIBRARY_DESC lib_desc_miss;
    lib_desc_miss.DXILLibrary.BytecodeLength = Shader::RaytracePass.Miss->GetBufferSize();
    lib_desc_miss.DXILLibrary.pShaderBytecode = Shader::RaytracePass.Miss->GetBufferPointer();
    lib_desc_miss.NumExports = misses.size();
    lib_desc_miss.pExports = misses.data();

    desc = {};
    desc.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    desc.pDesc = &lib_desc_miss;

    sub_objects[idx++] = desc;

    D3D12_EXPORT_DESC export_desc_closest_hit = {};
    export_desc_closest_hit.Name = L"ClosestHit_76";
    export_desc_closest_hit.ExportToRename = L"ClosestHit";
    export_desc_closest_hit.Flags = D3D12_EXPORT_FLAG_NONE;

    D3D12_DXIL_LIBRARY_DESC lib_desc_closest_hit;
    lib_desc_closest_hit.DXILLibrary.BytecodeLength = Shader::RaytracePass.Closest->GetBufferSize();
    lib_desc_closest_hit.DXILLibrary.pShaderBytecode = Shader::RaytracePass.Closest->GetBufferPointer();
    lib_desc_closest_hit.NumExports = 1;
    lib_desc_closest_hit.pExports = &export_desc_closest_hit;

    desc = {};
    desc.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
    desc.pDesc = &lib_desc_closest_hit;

    sub_objects[idx++] = desc;

    D3D12_HIT_GROUP_DESC hitGroupDesc = {};
    hitGroupDesc.ClosestHitShaderImport = L"ClosestHit_76";
    hitGroupDesc.HitGroupExport = L"HitGroup";

    D3D12_STATE_SUBOBJECT hitGroup = {};
    hitGroup.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
    hitGroup.pDesc = &hitGroupDesc;

    sub_objects[idx++] = hitGroup;

    D3D12_RAYTRACING_SHADER_CONFIG shaderDesc = {};
    shaderDesc.MaxPayloadSizeInBytes = sizeof(ShaderType::RayPayload);
    shaderDesc.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;

    D3D12_STATE_SUBOBJECT shaderConfigObject = {};
    shaderConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
    shaderConfigObject.pDesc = &shaderDesc;

    sub_objects[idx++] = shaderConfigObject;

    const WCHAR* shaderExports[] = { L"RayGen_12", L"Miss_5", L"HitGroup" , L"Miss_Shadow"};

    // Add a state subobject for the association between shaders and the payload
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION shaderPayloadAssociation = {};
    shaderPayloadAssociation.NumExports = _countof(shaderExports);
    shaderPayloadAssociation.pExports = shaderExports;
    shaderPayloadAssociation.pSubobjectToAssociate = &sub_objects[(idx - 1)];

    D3D12_STATE_SUBOBJECT shaderPayloadAssociationObject = {};
    shaderPayloadAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    shaderPayloadAssociationObject.pDesc = &shaderPayloadAssociation;

    sub_objects[idx++] = shaderPayloadAssociationObject;

    // Add a state subobject for the shared root signature
    D3D12_STATE_SUBOBJECT hitgroupRootSigObject = {};
    hitgroupRootSigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
    hitgroupRootSigObject.pDesc = _hit_group_root_signature.GetAddressOf();

    sub_objects[idx++] = hitgroupRootSigObject;

    // Create a list of the shader export names that use the root signature
    const WCHAR* rootSigExports[] = { L"HitGroup"};

    // Add a state subobject for the association between the RayGen shader and the local root signature
    D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION hitgroupShaderRootSigAssociation = {};
    hitgroupShaderRootSigAssociation.NumExports = _countof(rootSigExports);
    hitgroupShaderRootSigAssociation.pExports = rootSigExports;
    hitgroupShaderRootSigAssociation.pSubobjectToAssociate = &sub_objects[(idx - 1)];

    D3D12_STATE_SUBOBJECT hitgroupShaderRootSigAssociationObject = {};
    hitgroupShaderRootSigAssociationObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
    hitgroupShaderRootSigAssociationObject.pDesc = &hitgroupShaderRootSigAssociation;

    sub_objects[idx++] = hitgroupShaderRootSigAssociationObject;

    D3D12_STATE_SUBOBJECT globalRootSig;
    globalRootSig.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
    globalRootSig.pDesc = _global_root_signature.GetAddressOf();

    sub_objects[idx++] = globalRootSig;

    D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
    pipelineConfig.MaxTraceRecursionDepth = 2;

    D3D12_STATE_SUBOBJECT pipelineConfigObject = {};
    pipelineConfigObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
    pipelineConfigObject.pDesc = &pipelineConfig;

    sub_objects[idx++] = pipelineConfigObject;

    D3D12_STATE_OBJECT_DESC pipelineDesc = {};
    pipelineDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
    pipelineDesc.NumSubobjects = static_cast<UINT>(sub_objects.size());
    pipelineDesc.pSubobjects = sub_objects.data();

    // Create the RT Pipeline State Object (RTPSO)
    _rtpso = _device->CreateStateObject(&pipelineDesc);
    if (_rtpso == nullptr) {
        return false;
    }

    // Get the RTPSO properties
    HRESULT hr = _rtpso->QueryInterface(IID_PPV_ARGS(&_rtpso_info));
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

bool Raytracer::BuildResourceBuffer() {
    CBPerFrame null_frame;
    ShaderType::Material material;
    ShaderType::Light light;

    _cb_buffer_per_frames = std::move(std::make_unique<ConstantBuffer>(_device, Renderer::NumPreFrames));
    _cb_buffer_per_frames->SetData(&null_frame, sizeof(CBPerFrame));

    _rw_buffer_material = std::move(std::make_unique<RWResourceBuffer>(_device, Renderer::NumPreFrames));

    _rw_buffer_material->SetData(&material, MaterialMap::MaxMaterial,
                                 sizeof(ShaderType::Material));

    _rw_buffer_light = std::move(std::make_unique<RWResourceBuffer>(_device, Renderer::NumPreFrames));
    _rw_buffer_light->SetData(&light, RenderScreen::MAX_LIGHT, sizeof(ShaderType::Light));

    return true;
}

void Raytracer::Reset() {
    _integration_cnt = 0;
}

_END_ENGINE
