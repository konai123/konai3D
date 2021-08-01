//
// Created by korona on 2021-08-01.
//

#ifndef KONAI3D_RESOURCE_MAP_H
#define KONAI3D_RESOURCE_MAP_H

#include "src/engine/core/macros.h"
#include "src/engine/graphics/vertex_buffer.h"

_START_ENGINE
struct DrawInfo {
    UINT _index_count = 0;
    UINT _start_index_location = 0;
    UINT _base_vertex_location = 0;
    std::unique_ptr<VertexBuffer> _vertex_buffer;
    D3D_PRIMITIVE_TOPOLOGY _type;
};

template <typename TResourceName>
class ResourceMap {
DECLARE_CLASS_AS_INTERFACE(ResourceMap)

public:
    virtual bool Contains(std::string name) = 0;
    virtual TResourceName GetResource(std::string name) = 0;
    virtual std::string GetDefaultResourceName() = 0;
};

_END_ENGINE

#endif //KONAI3D_RESOURCE_MAP_H
