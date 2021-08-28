//
// Created by korona on 2021-08-23.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/shader_table.h"
#include "src/engine/graphics/constant_buffer.h"
#include "src/engine/graphics/rw_resource_buffer.h"
#include "src/engine/graphics/frame_resource_buffer.h"
#include "src/engine/graphics/material_map.h"
#include "src/engine/graphics/texture_map.h"
#include "src/engine/graphics/mesh_map.h"
#include "src/engine/graphics/tlas.h"
#include "src/engine/graphics/render_screen.h"

_START_ENGINE
class Raytracer {
public:
    struct CBPerObject {
        UINT MaterialIdx;
    };

    struct CBPerFrame {
        ShaderType::Camera Camera;
        UINT RenderTargetIdx;
        UINT TotalFrameCount;
        UINT IntegrationCount;
    };

    explicit Raytracer(std::shared_ptr<DeviceCom> deviceCom);
    virtual ~Raytracer() = default;

public:
    bool Initiate();

    void Render(float delta,
                RenderScreen * screen,
                ID3D12GraphicsCommandList6 *command_list,
                UINT currentFrameIndex,
                MeshMap *meshMap,
                TextureMap *textureMap,
                MaterialMap *materialMap,
                ResourceDescriptorHeap *heaps
    );

    void Reset();

public:
    inline static const UINT MAX_RENDER_OBJECT = 1000;

private:
    bool BuildGlobalRootSignature();
    bool BuildHitgroupRootSignature();
    bool BuildRSShaderTable(ID3D12GraphicsCommandList* cmdList);
    bool BuildRSPipelineState();
    bool BuildResourceBuffer();

    bool UpdateHitgroupTable(MeshMap* meshMap, MaterialMap* matMap, std::vector<RenderObject*> objs, UINT currentFrame, ID3D12GraphicsCommandList* cmdList);

private:
    Microsoft::WRL::ComPtr<ID3D12StateObject> _rtpso;
    Microsoft::WRL::ComPtr<ID3D12StateObjectProperties> _rtpso_info;

    std::unique_ptr<FrameResourceBuffer> _miss_shader_table_resource;
    std::unique_ptr<FrameResourceBuffer> _ray_gen_shader_table_resource;
    std::unique_ptr<FrameResourceBuffer> _hit_group_shader_table_resource;

    ShaderTable _hitgroup_table;
    ShaderTable _ray_gen_table;
    ShaderTable _miss_table;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> _global_root_signature;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _hit_group_root_signature;

    std::shared_ptr<DeviceCom> _device;

    std::unique_ptr<ConstantBuffer> _cb_buffer_per_frames;
    std::unique_ptr<RWResourceBuffer> _rw_buffer_material;
    TLAS _tlas;
    UINT _total_frame_cnt;
    UINT _integration_cnt;
};
_END_ENGINE
