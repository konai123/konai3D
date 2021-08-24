//
// Created by khhan on 2021-08-24.
//

#pragma once


_START_ENGINE
class Renderer;
class ResourceGarbageQueue {
    friend class Renderer;
public:
    virtual ~ResourceGarbageQueue() = default;
    ResourceGarbageQueue &operator=(const ResourceGarbageQueue &) = delete;
    ResourceGarbageQueue &operator=(ResourceGarbageQueue &&) noexcept = delete;
    ResourceGarbageQueue(const ResourceGarbageQueue &) = delete;
    ResourceGarbageQueue(ResourceGarbageQueue &&) noexcept = delete;

    static ResourceGarbageQueue& Instance();


public:
    void SubmitResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource);

protected:
    ResourceGarbageQueue() = default;

private:
    std::queue<Microsoft::WRL::ComPtr<ID3D12Resource>> WaitQ;

};
_END_ENGINE
