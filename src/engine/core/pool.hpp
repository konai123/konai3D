//
// Created by korona on 2021-06-13.
//


#pragma once

#include "src/engine/core/macros.h"

_START_ENGINE

/*
 * Thread free Pool
 * */
template<class T>
class Pool {
private:
    template<class T>
    struct PoolElement {
    public:
        T _object;
        int _next;
    };

    template<>
    struct PoolElement<void> {
    public:
        int _next;
    };

public:
    explicit Pool(int initialSize, bool autoIncrement) : _capacity(initialSize), _curr(0),
                                                         _auto_increment(autoIncrement) {
        _elements.resize(initialSize);
        clear();
    }

    Pool(const Pool &) = delete;
    Pool(Pool &&) noexcept = default;
    Pool &operator=(const Pool &) = delete;
    Pool &operator=(Pool &&) noexcept = default;
    auto &operator[](int idx) {
        return _elements[idx]._object;
    }

public:
    virtual int allocate() {
        std::lock_guard<std::mutex> lock(_lock);
        if (_curr >= _capacity) {
            if (!_auto_increment)
                return -1;

            _elements.emplace_back();
            _elements[_elements.size()-1]._next = _curr+1;
            _capacity++;
        }
        int ret = _curr;
        _curr = _elements[_curr]._next;
        _size++;
        return ret;
    }

    virtual void clear() {
        std::lock_guard<std::mutex> lock(_lock);
        _curr = 0;
        _size = 0;
        for (int i = 0; i < _elements.size(); i++) {
            _elements[i]._next = i + 1;
        }
    }

    virtual void free(int idx) {
        std::lock_guard<std::mutex> lock(_lock);
        _elements[idx]._next = _curr;
        _curr = idx;
        _size--;
    }

    virtual size_t size() const {
        return _size;
    }

private:
    std::mutex _lock;
    int _curr;
    bool _auto_increment;
    size_t _capacity;
    size_t _size;
    std::vector<PoolElement<T>> _elements;
};
_END_ENGINE
