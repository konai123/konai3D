//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/rendered.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
void Rendered::AddConstantBuffer(std::string attributeName, std::unique_ptr<ConstantBuffer> cbBuffer) {
    _cb_resource[attributeName] = std::move(cbBuffer);
}

ConstantBuffer* Rendered::GetConstantBuffer(std::string attributeName) {
    if (!_cb_resource.contains(attributeName)) {
        GRAPHICS_LOG_ERROR("cannot find constant resource '{}'", attributeName);
        return nullptr;
    }
    return _cb_resource[attributeName].get();
}

bool Rendered::UpdateConstantBuffer(std::string attributeName, void *data, UINT currentFrameIndex) {
    if (!_cb_resource.contains(attributeName)) {
        GRAPHICS_LOG_ERROR("cannot find constant resource '{}'", attributeName);
        return false;
    }
    return _cb_resource[attributeName]->UpdateData(data, currentFrameIndex);
}

_END_ENGINE
