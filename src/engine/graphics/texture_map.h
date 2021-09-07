//
// Created by korona on 2021-08-03.
//

#pragma once

#include "src/engine/graphics/device_com.h"
#include "src/engine/graphics/texture_loader.h"
#include "src/engine/graphics/resource_descriptor_heap.h"
#include "src/engine/graphics/resource_garbage_queue.h"
#include "src/engine/core/rwlock.hpp"

_START_ENGINE

struct TextureResource {
    std::filesystem::path Path;
    HeapDescriptorHandle Handle;
    Microsoft::WRL::ComPtr<ID3D12Resource> Resource;
};

class TextureMap {
public:
    explicit TextureMap(std::shared_ptr<DeviceCom> deviceCom, std::shared_ptr<ResourceDescriptorHeap> resourceHeap);
    virtual ~TextureMap();

public:
    void AsyncLoad(std::vector<std::filesystem::path> paths);
    std::vector<TextureResource> FetchTextureLoader(DirectX::ResourceUploadBatch* uploader);
    bool AddTexture(std::string key, TextureResource value);
    UINT UploadQueueSize();
    std::vector<std::string> GetTextureList();

public:
    bool Contains(std::string name);
    void Clear();
    std::optional<TextureResource> GetResource(std::string name);

private:
    std::optional<TextureResource> MakeTextureResource(Texture &&, DirectX::ResourceUploadBatch* uploader);

private:
    std::string _default_texture;
    std::shared_ptr<DeviceCom> _device;
    std::shared_ptr<ResourceDescriptorHeap> _resource_heap;
    std::unordered_map<std::string, TextureResource> _map;
    TextureLoader _texture_loader;
    RWLock _rw_lock;
};

_END_ENGINE
