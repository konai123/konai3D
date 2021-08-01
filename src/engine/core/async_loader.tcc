//
// Created by korona on 2021-07-31.
//

#include "src/engine/core/async_loader.h"
#include "src/engine/core/async_loader.h"
#include "src/engine/core/macros.h"

_START_ENGINE
template<typename TData>
AsyncLoader<TData>::AsyncLoader()
:
_is_busy(false) {}

template<typename TData>
AsyncLoader<TData>::~AsyncLoader() {
    CORE_LOG_INFO("Waiting Thread...");
    for (int i = 0; i < _threads.size(); i++) {
        _threads[i].join();
    }
}

template<typename TData>
std::vector<TData> AsyncLoader<TData>::Get() {
    std::vector<TData> v;
    std::lock_guard lock(_lock);
    while (!_loaded.empty()) {
        v.push_back(_loaded.front());
        _loaded.pop();
    }
    return v;
}

template<typename TData>
void AsyncLoader<TData>::Push(TData &&data) {
    std::lock_guard lock(_lock);
    _loaded.push(std::move(data));
}

template<typename TData>
void AsyncLoader<TData>::Load(std::vector<std::filesystem::path> paths) {
    auto future = std::thread([this](std::vector<std::filesystem::path> paths) {
        Delegate(paths);
    }, paths);
    _threads.push_back(std::move(future));
}

_END_ENGINE