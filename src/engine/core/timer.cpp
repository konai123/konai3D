//
// Created by korona on 2021-06-09.
//

#include "src/engine/core/timer.h"

_START_ENGINE
Timer::Timer()
:
_has_stopped(false) {
    Reset();
}

void Timer::Start() {
    if (!_has_stopped)
        return;

    ::QueryPerformanceCounter(&_curr_time);
    _has_stopped = false;
}

void Timer::Tick() {
    if (_has_stopped) {
        _delta_time = 0.0f;
        return;
    }

    _prev_time = _curr_time;
    ::QueryPerformanceCounter(&_curr_time);
    const double _second_per_sec = 1.0 / _timer_frequency.QuadPart;
    _delta_time = static_cast<float>((_curr_time.QuadPart - _prev_time.QuadPart) * _second_per_sec);
}

void Timer::Stop() {
    if (_has_stopped)
        return;

    ::QueryPerformanceCounter(&_prev_time);
    _has_stopped = true;
}

void Timer::Reset() {
    _timer_frequency = {0};
    _curr_time = {0};
    _prev_time = {0};
    _delta_time = 0.0f;
    ::QueryPerformanceFrequency(&_timer_frequency);
}

float Timer::DeltaTime() const {
    if (_delta_time < 0.0f)
        return 0.0f;

    return _delta_time;
}
_END_ENGINE

