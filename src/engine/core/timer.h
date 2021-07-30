//
// Created by korona on 2021-06-09.
//

#ifndef KONAI3D_TIMER_H
#define KONAI3D_TIMER_H

_START_ENGINE
class Timer {
public:
    Timer();
    virtual ~Timer() = default;

public:
    virtual void Start();
    virtual void Tick();
    virtual void Stop();
    virtual void Reset();
    virtual float DeltaTime() const;

private:
    LARGE_INTEGER _timer_frequency;
    LARGE_INTEGER _curr_time;
    LARGE_INTEGER _prev_time;

    float _delta_time;
    bool _has_stopped;
};
_END_ENGINE

#endif //KONAI3D_TIMER_H
