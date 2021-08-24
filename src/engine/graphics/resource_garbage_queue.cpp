//
// Created by khhan on 2021-08-24.
//

#include "src/engine/graphics/resource_garbage_queue.h"

_START_ENGINE
ResourceGarbageQueue &ResourceGarbageQueue::Instance() {
    static ResourceGarbageQueue inst;
    return inst;
}

void ResourceGarbageQueue::SubmitResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource) {
    WaitQ.push(resource);
}

_END_ENGINE
