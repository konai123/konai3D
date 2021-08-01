//
// Created by korona on 2021-07-28.
//

#ifndef KONAI3D_RENDERED_H
#define KONAI3D_RENDERED_H

#include "src/engine/graphics/constant_buffer.h"

_START_ENGINE
class Rendered {
public:
    void AddConstantBuffer(std::string attributeName, std::unique_ptr<ConstantBuffer> cbBuffer);
    ConstantBuffer* GetConstantBuffer(std::string attributeName);
    bool UpdateConstantBuffer(std::string attributeName, void *data, UINT currentFrameIndex);

private:
    std::unordered_map<std::string, std::unique_ptr<ConstantBuffer>> _cb_resource;
};

_END_ENGINE

#endif //KONAI3D_RENDERED_H
