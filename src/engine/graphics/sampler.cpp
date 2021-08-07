//
// Created by korona on 2021-07-30.
//

#include "src/engine/graphics/sampler.h"
#include "src/engine/graphics/macros.h"

_START_ENGINE
Sampler::Sampler(std::shared_ptr<DeviceCom> device, std::shared_ptr<ResourceDescriptorHeap> resource_heap_pool)
:
_device(std::move(device)),
_resource_heap(std::move(resource_heap_pool)) {}

Sampler::~Sampler() {
    if (_heap_desc.IsVaild())
        _resource_heap->DiscardSamplerHeapDescriptor(_heap_desc._heap_index);
}

bool Sampler::SetSamplerDesc(D3D12_SAMPLER_DESC desc) {
    _heap_desc = _resource_heap->AllocSamplerHeapDescriptor();
    if (!_device->CreateSamplerResource(&desc, _heap_desc.CpuHandle)) {
        GRAPHICS_LOG_ERROR("Failed to create sampler.");
        return false;
    }
    return true;
}

HeapDescriptorHandle *Sampler::GetHeapDescriptor() {
    return &_heap_desc;
}

_END_ENGINE