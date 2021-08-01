//
// Created by korona on 2021-08-01.
//

#ifndef KONAI3D_TEXTURE_H
#define KONAI3D_TEXTURE_H

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/resource_descriptor_heap.h"

_START_ENGINE
class Texture {
public:
    Texture(std::shared_ptr<DeviceCom> device,
            std::shared_ptr<ResourceDescriptorHeap> resourceHeap,
            DirectX::ScratchImage image,
            DirectX::ResourceUploadBatch* uploader
            );
    virtual ~Texture();

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateShaderResource(DirectX::ResourceUploadBatch* uploader);

private:
    std::shared_ptr<DeviceCom> _device;
    std::shared_ptr<ResourceDescriptorHeap> _resource_heap;
    Microsoft::WRL::ComPtr<ID3D12Resource> _resource;
    HeapDescriptor _resource_view;
    DirectX::ScratchImage _image;
};

_END_ENGINE

#endif //KONAI3D_TEXTURE_H
