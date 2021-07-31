//
// Created by korona on 2021-07-31.
//

#include "src/engine/core/async_loader.h"

_START_ENGINE
template<typename TData>
AsyncLoader<TData>::AsyncLoader()
        :
        _is_busy(false) {}

template<typename TData>
std::vector<TData> AsyncLoader<TData>::Get() {
    if (IsBusy()) {
        return std::vector<TData>();
    }

    std::vector<TData> v;
    std::lock_guard lock(_lock);
    while (!_loaded.empty()) {
        v.push_back(_loaded.front());
        _loaded.pop();
    }
    return v;
}

template<typename TData>
void AsyncLoader<TData>::Push(const TData &&data) {
    std::lock_guard lock(_lock);
    _loaded.push(data);
}

template<typename TData>
bool AsyncLoader<TData>::Load(std::vector<std::string> paths) {
    if (IsBusy()) return false;
    std::async(std::launch::async, [&, this]() {
        _is_busy.store(true);
        Delegate(paths);
        _is_busy.store(false);
    });
    return true;
}

template<typename TData>
bool AsyncLoader<TData>::IsBusy() {
    return _is_busy.load();
}

template<typename TData>
void AsyncLoader<TData>::Wait() {
    while (_is_busy.load()) {
        Sleep(10);
    }
}

_END_ENGINE