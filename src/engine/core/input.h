//
// Created by korona on 2021-06-11.
//


#pragma once

_START_ENGINE
struct MousePosition {
public:
    MousePosition(int x, int y) : _x(x), _y(y) {}
    MousePosition() : _x(0), _y(0) {}

public:
    int _x, _y;
};

class Input {
public:
    Input();
    virtual ~Input() = default;

public:
    virtual bool Initiate(HWND hwnd);
    virtual void HandleMessage(const MSG *msg);

public:
    void Reset();
    bool GetKeyboardButton(const int code) const;
    bool GetKeyboardButtonDown(const int code) const;
    bool GetKeyboardButtonUp(const int code) const;
    bool GetMouseButton(const int code) const;
    bool GetMouseButtonDown(const int code) const;
    bool GetMouseButtonUp(const int code) const;
    int GetMouseWheelDelta() const;
    MousePosition GetMousePosition() const;

public:
    void SetKeyboardButtonUp(const int code);
    void SetKeyboardButtonDown(const int code);
    void SetMouseButtonUp(const int code);
    void SetMouseButtonDown(const int code);
    void SetMouseWheelDelta(const int delta);
    void SetMousePosition(const int xPosition, const int yPosition);
    void Update();

private:
    std::array<bool, 0xFE> _curr_key_state;
    std::array<bool, 0xFE> _prev_key_state;
    int _wheel_delta;
    int _x_position;
    int _y_position;

};
_END_ENGINE