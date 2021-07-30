//
// Created by khhan on 2021-06-17.
//

#ifndef KONAI3D_GLOBAL_H
#define KONAI3D_GLOBAL_H

/*externals*/
_START_KONAI3D
namespace global
{
extern std::filesystem::path ExecutablePath;
extern std::filesystem::path AssetPath;
extern std::filesystem::path ShaderPath;

tString GetErrorMessageFromHR(HRESULT hr);
}//end namespace global
_END_KONAI3D
#endif //KONAI3D_GLOBAL_H