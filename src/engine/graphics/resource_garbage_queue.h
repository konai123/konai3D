//
// Created by khhan on 2021-08-24.
//

#pragma once


_START_ENGINE
class Renderer;
class ResourceGarbageQueue {
    friend class Renderer;
public:
    virtual ~ResourceGarbageQueue();
    ResourceGarbageQueue &operator=(const ResourceGarbageQueue &) = delete;
    ResourceGarbageQueue &operator=(ResourceGarbageQueue &&) noexcept = delete;
    ResourceGarbageQueue(const ResourceGarbageQueue &) = delete;
    ResourceGarbageQueue(ResourceGarbageQueue &&) noexcept = delete;

    static ResourceGarbageQueue& Instance();


public:
    void SubmitResource(Microsoft::WRL::ComPtr<ID3D12Resource> resource);
    void Flush();

public:
    inline static bool Activate = false;

protected:
    ResourceGarbageQueue() = default;

private:
    std::queue<Microsoft::WRL::ComPtr<ID3D12Resource>> WaitQ;

};
_END_ENGINE
