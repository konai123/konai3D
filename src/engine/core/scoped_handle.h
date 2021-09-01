//
// Created by korona on 2021-09-01.
//

#pragma once

class ScopedHandle
{
public:
    explicit ScopedHandle(HANDLE handle) {
        Handle = handle;
    }

    ~ScopedHandle() {
        ::CloseHandle(Handle);
    }

    HANDLE Get() {
        return Handle;
    }

public:
    HANDLE Handle;
};