//
// Created by khhan on 2021-06-21.
//

#pragma once

#include "src/engine/core/macros.h"

_START_ENGINE
class UIRenderer
{
DECLARE_CLASS_AS_INTERFACE(UIRenderer);

public:
    virtual bool Initiate(
            const HWND hwnd,
            DeviceCom *device,
            const DXGI_FORMAT backbufferFormat,
            ID3D12DescriptorHeap *srvDescriptorHeap,
            HeapDescriptorHandle *shaderResourceView,
            const UINT appWidth,
            const UINT appHeight,
            const UINT numFrameFlight
    )=0;
    virtual void OnGUIRender(float delta, ID3D12GraphicsCommandList* cmd_list) = 0;
    virtual void OnResize(UINT width, UINT height) = 0;
    virtual void OnDestroy() = 0;
};
_END_ENGINE
