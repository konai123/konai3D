//
// Created by korona on 2021-08-04.
//

#pragma once

#include "src/engine/graphics/shader_types.h"
#include "src/engine/core/pool.hpp"

_START_ENGINE

struct MaterialDesc {
public:
    std::string DiffuseTexturePath;
    bool Dirty = true;
};

class MaterialMap {
public:
    MaterialMap();

public:
    bool AddMaterial(std::string name, MaterialDesc descriptor);
    bool UpdateMaterial(std::string name, MaterialDesc descriptor);
    void DeleteMaterial(std::string name);
    int GetMaterialID(std::string name);
    std::optional<MaterialDesc> GetMaterialDesc(std::string name);
    std::vector<std::string> GetMaterialList();

public:
    inline static const UINT MaxMaterial = 1000;

private:
    std::unordered_map<std::string, std::pair<MaterialDesc, int>> _map;
    Pool<void> _pool;
};

_END_ENGINE
