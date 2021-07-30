//
// Created by khhan on 2021-06-17.
//

#include "src/global.h"

_START_KONAI3D
namespace global{
std::filesystem::path ExecutablePath;
std::filesystem::path AssetPath;
std::filesystem::path ShaderPath;

tString GetErrorMessageFromHR(HRESULT hr) {
    _com_error err(hr);
    tString msg = err.ErrorMessage();
    return msg;
}
}//end namespace
_END_KONAI3D