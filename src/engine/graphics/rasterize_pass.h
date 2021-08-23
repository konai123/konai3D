//
// Created by korona on 2021-08-02.
//

#pragma once

#include "src/engine/graphics/render_screen.h"
#include "src/engine/graphics/constant_buffer.h"
#include "src/engine/graphics/rw_resource_buffer.h"
#include "src/engine/graphics/mesh_map.h"
#include "src/engine/graphics/texture_map.h"
#include "src/engine/graphics/material_map.h"
#include "src/engine/graphics/shader_types.h"
#include "src/engine/core/pool.hpp"

_START_ENGINE

class Rasterizer {
public:
    struct CBPerFrame {
        float4x4 view_mat;
        float4x4 inverse_view_mat;
        float4x4 projection_mat;
    };

    struct CBPerObject {
        float4x4 world_mat;
        int material_id;
    };

public:
    Rasterizer(std::shared_ptr<DeviceCom> deviceCom);
    virtual ~Rasterizer() = default;

public:
    bool Initiate();

    void Render(float delta,
                std::vector<RenderScreen *> screens,
                ID3D12GraphicsCommandList *command_list,
                UINT currentFrameIndex,
                MeshMap *meshMap,
                TextureMap *textureMap,
                MaterialMap *materialMap,
                ResourceDescriptorHeap *heaps
    );

public:
    inline static const UINT MAX_RENDER_OBJECT = 1000;

private:
    bool BuildRootSignature();
    bool BuildPipelineState();
    bool BuildResourceBuffer();

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> _root_signature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> _opaque_pso;

    std::shared_ptr<DeviceCom> _device;

    std::unique_ptr<RWResourceBuffer> _rw_buffer_material;
    std::unique_ptr<ConstantBuffer> _cb_buffer_per_frames;
    std::array<std::unique_ptr<ConstantBuffer>, MAX_RENDER_OBJECT> _cb_buffer_per_objects;

};
_END_ENGINE
