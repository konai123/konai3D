//
// Created by korona on 2021-08-23.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/rw_resource_buffer.h"
#include "src/engine/graphics/resource_garbage_queue.h"

_START_ENGINE
struct ShaderRecord {
    void AddShaderID(void* IDBuffer) {
        std::vector<UINT8> copied;
        for (int i = 0; i < D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; i++) {
            copied.push_back(reinterpret_cast<UINT8*>(IDBuffer)[i]);
        }
        Data.push_back(copied);
        RecordeSize += D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    }

    template<typename T, UINT Tsize>
    void AddField(T pData) {
        std::vector<UINT8> dest(Tsize);
        memcpy(dest.data(), &pData, Tsize);
        RecordeSize += Tsize;
        Data.push_back(dest);
    }

    UINT RecordeSize = 0;
    std::vector<std::vector<UINT8>> Data;
};

struct ShaderTable {
    void AddRecord(const ShaderRecord& record) {
        MaxRecordSize = max(MaxRecordSize, ALIGN(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, record.RecordeSize));
        Records.push_back(record);
    }

    UINT GetBytesSize() {
        return ALIGN(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, MaxRecordSize * Records.size());
    }

    bool Generate(RWResourceBuffer* buffer, UINT updateIdx, UINT currentFrame) {
        std::vector<UINT8> raw(GetBytesSize());
        for (auto& record : Records) {
            UINT offset = 0;
            for (auto p : record.Data) {
                std::copy(p.begin(), p.end(), raw.begin()+offset);
                offset += p.size();
            }
            offset = ALIGN(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT, offset);
        }
        buffer->UpdateData(raw.data(), updateIdx, currentFrame);
        return true;
    }

    void Clear() {
        MaxRecordSize = 0;
        Records.clear();
    }

    UINT MaxRecordSize = 0;
    std::vector<ShaderRecord> Records;
};
_END_ENGINE
