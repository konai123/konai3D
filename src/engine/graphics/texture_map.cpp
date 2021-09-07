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

std::vector<TextureResource> TextureMap::FetchTextureLoader(DirectX::ResourceUploadBatch* uploader) {
    std::vector<TextureResource> resources;
    auto v = _texture_loader.Get();
    for (int i = 0; i < v.size(); i++) {
        auto ret = MakeTextureResource(std::move(v[i]), uploader);
        if (ret.has_value()) {
            resources.push_back(ret.value());
        }
    }
    return resources;
}

bool TextureMap::AddTexture(std::string key, TextureResource value) {
    LocalWriteLock lock(_rw_lock);
    if (_map.contains(key)) {
        GRAPHICS_LOG_ERROR("texture '{}' already registered", key);
        return false;
    }
    _map[key] = value;
    return true;
}

std::optional<TextureResource> TextureMap::MakeTextureResource(Texture &&texture, DirectX::ResourceUploadBatch* uploader) {
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
        return std::nullopt;
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

    TextureResource ret = {.Path = texture.path, .Handle = heapDescriptor, .Resource = resource};

    return ret;
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

UINT TextureMap::UploadQueueSize() {
    return _texture_loader.Size();
}

void TextureMap::Clear() {
    LocalWriteLock lock(_rw_lock);
    _texture_loader.Get();

    for (auto &p : _map) {
        if (p.second.Handle.IsVaild()) {
            _resource_heap->DiscardShaderResourceHeapDescriptor(p.second.Handle._heap_index);
            if (p.second.Resource != nullptr) {
                ResourceGarbageQueue::Instance().SubmitResource(p.second.Resource);
            }
        }
    }
    _map.clear();
}

std::optional<TextureResource> TextureMap::GetResource(std::string name) {
    LocalReadLock lock(_rw_lock);
    if (!_map.contains(name)) {
        return std::nullopt;
    }
    return _map[name];
}

_END_ENGINE

