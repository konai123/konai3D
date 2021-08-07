//
// Created by korona on 2021-08-04.
//

#include "src/engine/graphics/material_map.h"
#include "src/engine/graphics/macros.h"


_START_ENGINE

MaterialMap::MaterialMap()
:
_pool(MaxMaterial, false)
{}

bool MaterialMap::AddMaterial(std::string name, engine::MaterialDesc descriptor) {
    if (_map.contains(name)) {
        GRAPHICS_LOG_ERROR("Material name : {} already registered", name);
        return false;
    }

    int idx = _pool.allocate();
    _map[name] = std::make_pair(descriptor, idx);
    return true;
}

void MaterialMap::DeleteMaterial(std::string name) {
    if (!_map.contains(name)) {
        GRAPHICS_LOG_ERROR("Unregistered material : {}", name);
        return;
    }
    _pool.free(_map[name].second);
    _map.erase(name);
}

int MaterialMap::GetMaterialID(std::string name) {
    if (!_map.contains(name)) {
        GRAPHICS_LOG_ERROR("Unregistered material : {}", name);
        return -1;
    }
    return _map[name].second;
}

std::optional<MaterialDesc> MaterialMap::GetMaterialDesc(std::string name) {
    if (!_map.contains(name)) {
        GRAPHICS_LOG_ERROR("Unregistered material : {}", name);
        return std::nullopt;
    }
    return _map[name].first;
}

std::vector<std::string> MaterialMap::GetMaterialList() {
    std::vector<std::string> names;
    for (auto& m : _map) {
        names.push_back(m.first);
    }
    return names;
}

_END_ENGINE

