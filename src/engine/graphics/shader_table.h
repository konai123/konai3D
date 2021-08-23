//
// Created by korona on 2021-08-23.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/rw_resource_buffer.h"
#include "src/engine/graphics/renderer.h"

_START_ENGINE
struct ShaderRecord {
    template<typename T, UINT Tsize>
    void AddField(T* pData) {
        data.push_back(std::make_pair(pData, Tsize));
        RecordeSize += Tsize;
    }

    UINT RecordeSize = 0;
    std::vector<std::pair<void*, UINT>> data;
};

struct ShaderTable {
    void AddRecord(const ShaderRecord& record) {
        MaxRecordSize = max(MaxRecordSize, ALIGN(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, record.RecordeSize));
        Records.push_back(record);
    }

    UINT GetBytesSize() {
        return MaxRecordSize * Records.size();
    }

    std::unique_ptr<RWResourceBuffer> Generate(std::shared_ptr<DeviceCom> device) {
        auto resource = std::make_unique<RWResourceBuffer>(device, Renderer::NumPreFrames);
        resource->SetData(Records.size(), MaxRecordSize);

        UINT idx = 0;
        for (auto& record : Records) {
            std::vector<UINT8> raw(MaxRecordSize);
            UINT offset = 0;
            for (auto p : record.data) {
                UINT sz = p.second;
                UINT8* pointer = reinterpret_cast<UINT8*>(p.first);
                raw.insert(raw.begin() + offset, pointer, pointer+sz);
                offset += sz;
            }
            for (UINT j = 0; j < Renderer::NumPreFrames; j++)
            {
                resource->UpdateData(raw.data(), idx, j);
            }
            idx++;
        }

        return std::move(resource);
    }

    UINT MaxRecordSize = 0;
    std::vector<ShaderRecord> Records;
};
_END_ENGINE
