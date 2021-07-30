//
// Created by korona on 2021-06-17.
//

#ifndef KONAI3D_LOG_WINDOW_H
#define KONAI3D_LOG_WINDOW_H

#include "src/editor/imgui_window.h"
#include "src/engine/core/rwlock.hpp"

_START_KONAI3D
class LogWindow : public IMGUIWindow {
public:
    LogWindow();
    virtual ~LogWindow() = default;

public:
    bool Initiate();

public:
    virtual void OnUpdate(float delta) override;
    virtual void OnDestroy() override;

public:
    void Clear();

private:
    class SpdlogSink : public spdlog::sinks::base_sink<std::mutex> {
    public:
        SpdlogSink() {
            _line_offset.push_back(0);
        }

        /*
         * spdlog::sinks overrides
         * */
    public:
        // reference: https://spdlog.docsforge.com/v1.x/4.sinks/#implementing-your-own-sink
        virtual void sink_it_(const spdlog::details::log_msg &msg) override {
            // log_msg is a struct containing the log entry info like level, timestamp, thread id etc.
            // msg.raw contains pre formatted log

            // If needed (very likely but not mandatory), the sink formats the message before sending it to its final destination:
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);
            std::string log = fmt::to_string(formatted);

            _ENGINE::LocalWriteLock rwlock(_rw_lock);
            int old = _buffer.size();
            _buffer.append(log.data());
            for (int i = old; i < _buffer.size(); i++) {
                if (_buffer[i] == '\n')
                    _line_offset.push_back(i + 1);
            }

        }

        virtual void flush_() override {}

    public:
        ImGuiTextBuffer _buffer;
        ImVector<int> _line_offset;
        _ENGINE::RWLock _rw_lock;
    };

private:
    std::shared_ptr<SpdlogSink> _log_sink;
    ImGuiTextFilter _filter;
    ImFont *_logFont;
    bool _auto_scroll;
};
_END_KONAI3D

#endif //KONAI3D_LOG_WINDOW_H
