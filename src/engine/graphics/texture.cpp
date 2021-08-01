//
// Created by korona on 2021-08-01.
//

#include "src/engine/graphics/texture.h"

#include <utility>

_START_ENGINE
Texture::Texture(std::shared_ptr<DeviceCom> device,
                 std::shared_ptr<ResourceDescriptorHeap> resourceHeap,
                 DirectX::ScratchImage image,
                 DirectX::ResourceUploadBatch *uploader
)
:
_device(std::move(device)),
_resource_heap(std::move(resourceHeap)),
_resource(nullptr),
_image(std::move(image)) {
    _resource = CreateShaderResource(uploader);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Texture::CreateShaderResource(DirectX::ResourceUploadBatch *uploader) {
    CD3DX12_RESOURCE_DESC desc;

    auto &metadata = _image.GetMetadata();

    switch (metadata.dimension) {
        case DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE1D:
            desc = CD3DX12_RESOURCE_DESC::Tex1D(
                    metadata.format,
                    static_cast<UINT64>(metadata.format),
                    static_cast<UINT16>(metadata.arraySize)
            );
            break;

        case DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE2D:
            desc = CD3DX12_RESOURCE_DESC::Tex2D(
                    metadata.format,
                    static_cast<UINT64>(metadata.width),
                    static_cast<UINT>(metadata.height),
                    static_cast<UINT16>(metadata.arraySize)
            );
            break;

        case DirectX::TEX_DIMENSION::TEX_DIMENSION_TEXTURE3D:
            desc = CD3DX12_RESOURCE_DESC::Tex3D(
                    metadata.format,
                    static_cast<UINT64>(metadata.width),
                    static_cast<UINT>(metadata.height),
                    static_cast<UINT16>(metadata.depth)
            );
            break;
    }

    std::vector<D3D12_SUBRESOURCE_DATA> subresources(_image.GetImageCount());
    auto images = _image.GetImages();

    for (UINT i = 0; i < _image.GetImageCount(); i++) {
        subresources[i].RowPitch = images[i].rowPitch;
        subresources[i].SlicePitch = images[i].slicePitch;
        subresources[i].pData = images[i].pixels;
    }

    auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto resource = _device->CreateResource(&properties, &desc, nullptr, D3D12_HEAP_FLAG_NONE,
                                            D3D12_RESOURCE_STATE_COMMON);

    EngineAssert(resource.Get() != nullptr);
    uploader->Upload(resource.Get(), 0, subresources.data(), subresources.size());

    _resource_view = _resource_heap->GetShaderResourceHeapDescriptor();
    if (!_resource_view.IsVaild()) {
        GRAPHICS_LOG_ERROR("Failed to allocate shader resource heap.");
        return nullptr;
    }

    _device->CreatDescriptorHeapView<D3D12_SHADER_RESOURCE_VIEW_DESC>(
            resource.Get(), nullptr, _resource_view._cpu_handle, nullptr
    );
    return resource;
}

Texture::~Texture() {
    if (_resource_view.IsVaild())
        _resource_heap->DiscardShaderResourceHeapDescriptor(_resource_view._heap_index);
}

_END_ENGINE
