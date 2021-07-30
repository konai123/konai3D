//
// Created by korona on 2021-06-11.
//

#include "src/engine/core/input.h"
#include "src/engine/core/macros.h"

_START_ENGINE
Input::Input()
:
_wheel_delta(0),
_x_position(0),
_y_position(0) {
    Reset();
}

bool Input::Initiate(HWND hwnd) {
    return true;
}

void Input::HandleMessage(const MSG *msg) {
    switch (msg->message) {
        case WM_KEYDOWN:
            SetKeyboardButtonDown(static_cast<int>(msg->wParam));
            break;
        case WM_KEYUP:
            SetKeyboardButtonUp(static_cast<int>(msg->wParam));
            break;
        case WM_LBUTTONDOWN:
            SetMouseButtonDown(VK_LBUTTON);
            break;
        case WM_MBUTTONDOWN:
            SetMouseButtonDown(VK_MBUTTON);
            break;
        case WM_RBUTTONDOWN:
            SetMouseButtonDown(VK_RBUTTON);
            break;
        case WM_LBUTTONUP:
            SetMouseButtonUp(VK_LBUTTON);
            break;
        case WM_MBUTTONUP:
            SetMouseButtonUp(VK_MBUTTON);
            break;
        case WM_RBUTTONUP:
            SetMouseButtonUp(VK_RBUTTON);
            break;
        case WM_MOUSEMOVE: {
            const POINTS point = MAKEPOINTS(msg->lParam);
            SetMousePosition(point.x, point.y);
            break;
        }
        case WM_MOUSEWHEEL: {
            short delta = GET_WHEEL_DELTA_WPARAM(msg->wParam);
            SetMouseWheelDelta(delta);
            break;
        }
        case WM_KILLFOCUS:
            Reset();
            break;
    }
}

void Input::Reset() {
    std::fill(_curr_key_state.begin(), _curr_key_state.end(), false);
    std::fill(_prev_key_state.begin(), _prev_key_state.end(), false);
    _x_position = 0;
    _y_position = 0;
    _wheel_delta = 0;
}

bool Input::GetKeyboardButton(const int code) const {
    return _curr_key_state[code];
}

bool Input::GetKeyboardButtonDown(const int code) const {
    if (!_prev_key_state[code] && _curr_key_state[code])
        return true;
    return false;
}

bool Input::GetKeyboardButtonUp(const int code) const {
    if (_prev_key_state[code] && !_curr_key_state[code])
        return true;
    return false;
}

bool Input::GetMouseButton(const int code) const {
    return _curr_key_state[code];
}

bool Input::GetMouseButtonDown(const int code) const {
    if (!_prev_key_state[code] && _curr_key_state[code])
        return true;
    return false;
}

bool Input::GetMouseButtonUp(const int code) const {
    if (_prev_key_state[code] && !_curr_key_state[code])
        return true;
    return false;
}


int Input::GetMouseWheelDelta() const {
    return _wheel_delta;
}

MousePosition Input::GetMousePosition() const {
    return {_x_position, _y_position};
}

void Input::SetKeyboardButtonUp(const int code) {
    _curr_key_state[code] = false;
}

void Input::SetKeyboardButtonDown(const int code) {
    _curr_key_state[code] = true;
}

void Input::SetMouseButtonUp(const int code) {
    _curr_key_state[code] = false;
}

void Input::SetMouseButtonDown(const int code) {
    _curr_key_state[code] = true;
}

void Input::SetMouseWheelDelta(const int delta) {
    if (delta > 0) _wheel_delta = 1;
    else if (delta < 0) _wheel_delta = -1;
    else _wheel_delta = 0;
}

void Input::SetMousePosition(const int xPosition, const int yPosition) {
    _x_position = xPosition;
    _y_position = yPosition;
}

void Input::Update() {
    std::copy(_curr_key_state.begin(), _curr_key_state.end(), _prev_key_state.begin());
    _wheel_delta = 0;
}
_END_ENGINE