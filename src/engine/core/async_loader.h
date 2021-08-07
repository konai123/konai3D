//
// Created by korona on 2021-07-31.
//

#pragma once

_START_ENGINE
template <typename TData>
class AsyncLoader{
public:
    AsyncLoader();
    virtual ~AsyncLoader();
    AsyncLoader &operator=(const AsyncLoader &) = delete;
    AsyncLoader &operator=(AsyncLoader &&) noexcept = default;
    AsyncLoader(const AsyncLoader &) = delete;
    AsyncLoader(AsyncLoader &&) noexcept = default;

public:
    virtual void Delegate(std::vector<std::filesystem::path> paths) = 0;

public:
    std::vector<TData> Get();
    void AsyncLoad(std::vector<std::filesystem::path> paths);

protected:
    void Push(TData&&);

private:
    std::vector<std::thread> _threads;
    std::queue<TData> _loaded;
    std::atomic<bool> _is_busy;
    std::mutex _lock;
};

_END_ENGINE

#include "src/engine/core/async_loader.tcc"
