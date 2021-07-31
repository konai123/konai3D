//
// Created by korona on 2021-07-31.
//

#ifndef KONAI3D_ASYNC_LOADER_H
#define KONAI3D_ASYNC_LOADER_H


_START_ENGINE
template <typename TData>
class AsyncLoader{
public:
    AsyncLoader();
    virtual ~AsyncLoader() = default;
    AsyncLoader &operator=(const AsyncLoader &) = delete;
    AsyncLoader &operator=(AsyncLoader &&) noexcept = default;
    AsyncLoader(const AsyncLoader &) = delete;
    AsyncLoader(AsyncLoader &&) noexcept = default;

public:
    virtual void Delegate(std::vector<std::string> paths) = 0;

public:
    std::vector<TData> Get();
    bool Load(std::vector<std::string> paths);
    bool IsBusy();
    void Wait();

protected:
    void Push(const TData&&);

private:
    std::queue<TData> _loaded;
    std::atomic<bool> _is_busy;
    std::mutex _lock;
};

_END_ENGINE

#include "src/engine/core/async_loader.tcc"

#endif //KONAI3D_ASYNC_LOADER_H
