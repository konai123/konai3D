//
// Created by khhan on 2021-06-16.
//

#ifndef KONAI3D_COM_EXCEPTION_HPP
#define KONAI3D_COM_EXCEPTION_HPP

_START_ENGINE
class ComException : public std::exception {
public:
    ComException(HRESULT hr) : _hr(hr) {}

    [[nodiscard]] const char *what() const noexcept override {
        std::ostringstream oss;
        oss << "[COM Exception]::" << static_cast<unsigned int>(_hr);
        return oss.str().data();
    }

private:
    HRESULT _hr;
};
_END_ENGINE

#endif //KONAI3D_COM_EXCEPTION_HPP
