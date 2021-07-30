//
// Created by khhan on 2021-06-21.
//

#ifndef KONAI3D_UI_RENDERER_H
#define KONAI3D_UI_RENDERER_H

#include "src/engine/core/macros.h"

_START_ENGINE
struct RenderOutput;
class UIRenderer
{
DECLARE_CLASS_AS_INTERFACE(UIRenderer);

public:
    virtual bool Initiate(
            const HWND hwnd,
            DeviceCom *device,
            const DXGI_FORMAT backbufferFormat,
            ID3D12DescriptorHeap *srvDescriptorHeap,
            HeapDescriptor *shaderResourceView,
            const UINT appWidth,
            const UINT appHeight,
            const UINT numFrameFlight
    )=0;
    virtual void OnGUIRender(float delta, ID3D12GraphicsCommandList* cmd_list) = 0;
    virtual void OnResize(UINT width, UINT height) = 0;
    virtual void OnDestroy() = 0;
};
_END_ENGINE
#endif //KONAI3D_UI_RENDERER_H
