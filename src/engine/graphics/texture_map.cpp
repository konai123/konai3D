//
// Created by korona on 2021-08-03.
//

#include "src/engine/graphics/texture_map.h"

_START_ENGINE
TextureMap::TextureMap(
std::shared_ptr<DeviceCom> deviceCom,
std::shared_ptr<ResourceDescriptorHeap> resourceHeap
)
:
_device(deviceCom),
_resource_heap(resourceHeap)
{
}

TextureMap::~TextureMap() {
    for (auto &p : _map) {
        if (p.second.Handle.IsVaild()) {
            _resource_heap->DiscardShaderResourceHeapDescriptor(p.second.Handle._heap_index);
        }
    }
}

void TextureMap::AsyncLoad(std::vector<std::filesystem::path> paths) {
    _texture_loader.AsyncLoad(paths);
}

void TextureMap::UpdateFromTextureLoader(DirectX::ResourceUploadBatch* uploader) {
    auto v = _texture_loader.Get();
    AddTexture(std::move(v), uploader);
}

void TextureMap::AddTexture(std::vector<Texture> &&textures, DirectX::ResourceUploadBatch* uploader) {
    for (auto &&texture : textures) {

        LocalWriteLock lock(_rw_lock);
        if (_map.contains(texture.path.string())) {
            GRAPHICS_LOG_ERROR("texture '{}' already registered", texture.path.string());
            continue;
        }

        DirectX::ScratchImage image = std::move(texture.image);
        DirectX::TexMetadata meta = image.GetMetadata();

        CD3DX12_RESOURCE_DESC desc;
        switch (meta.dimension) {
            case DirectX::TEX_DIMENSION_TEXTURE1D:
                desc = CD3DX12_RESOURCE_DESC::Tex1D(
                        meta.format,
                        static_cast<UINT64>(meta.width),
                        static_cast<UINT16>(meta.arraySize)
                );
                break;
            case DirectX::TEX_DIMENSION_TEXTURE2D:
                desc = CD3DX12_RESOURCE_DESC::Tex2D(
                        meta.format,
                        static_cast<UINT64>(meta.width),
                        static_cast<UINT>(meta.height)
                );
                break;
            case DirectX::TEX_DIMENSION_TEXTURE3D:
                desc = CD3DX12_RESOURCE_DESC::Tex3D(
                        meta.format,
                        static_cast<UINT64>(meta.width),
                        static_cast<UINT>(meta.height),
                        static_cast<UINT16>(meta.depth)
                );
                break;
        }

        auto properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto resource = _device->CreateResource(&properties, &desc, nullptr, D3D12_HEAP_FLAG_NONE,
                                                D3D12_RESOURCE_STATE_COMMON);
        if (resource == nullptr) {
            GRAPHICS_LOG_ERROR("Cannot load texture: {}", texture.path.string());
        }

        auto cnt = image.GetImageCount();
        auto images = image.GetImages();
        auto subresource = std::vector<D3D12_SUBRESOURCE_DATA>(cnt);

        for (UINT i = 0; i < cnt; i++) {
            subresource[i].pData = images[i].pixels;
            subresource[i].SlicePitch = images[i].slicePitch;
            subresource[i].RowPitch = images[i].rowPitch;
        }
        uploader->Upload(resource.Get(), 0, subresource.data(), static_cast<UINT>(subresource.size()));
        auto heapDescriptor = _resource_heap->AllocShaderResourceHeapDescriptor();
        _device->CreatDescriptorHeapView<D3D12_SHADER_RESOURCE_VIEW_DESC>(
                resource.Get(), nullptr, heapDescriptor.CpuHandle, nullptr
        );

        _map[texture.path.string()] = { .Handle = heapDescriptor, .Resource = resource};
    }
}

std::vector<std::string> TextureMap::GetTextureList() {
    LocalReadLock lock(_rw_lock);
    std::vector<std::string> names;
    for (auto &p : _map) {
        names.push_back(p.first);
    }
    return names;
}

bool TextureMap::Contains(std::string name) {
    LocalReadLock lock(_rw_lock);
    return _map.contains(name);
}

std::optional<TextureResource> TextureMap::GetResource(std::string name) {
    LocalReadLock lock(_rw_lock);
    if (!_map.contains(name)) {
        GRAPHICS_LOG_ERROR("Cannot find texture: {}", name);
        return std::nullopt;
    }
    return _map[name];
}

_END_ENGINE

