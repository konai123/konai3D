//
// Created by korona on 2021-06-17.
//

#include "src/engine/core/logger.hpp"
#include "src/engine/graphics/macros.h"
#include "src/editor/log_window.h"
#include "src/macros.h"

_START_KONAI3D
LogWindow::LogWindow()
:
IMGUIWindow("Log")
{}

bool LogWindow::Initiate() {
    _log_sink = std::make_shared<SpdlogSink>();
    _ENGINE::Logger::AddSink<std::mutex>(_log_sink, APP_LOG_FILE_PATH);
    _ENGINE::Logger::AddSink<std::mutex>(_log_sink, GRAPHICS_LOG_FILE_PATH);

    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig config;
    _logFont = io.Fonts->AddFontDefault(&config);
    return true;
}

void LogWindow::OnUpdate(float delta) {
    std::string name = GetWindowName();
    if (!ImGui::Begin(name.c_str(), &_open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        Clear();
    }

    ImGui::SameLine();
    if (ImGui::BeginPopup("Options")) {
        ImGui::Checkbox("Auto-scroll", &_auto_scroll);
        ImGui::EndPopup();
    }

    if (ImGui::Button("Options"))
        ImGui::OpenPopup("Options");

    _filter.Draw();

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGuiCol_MenuBarBg);
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::Selectable("Clear")) Clear();
        ImGui::EndPopup();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushFont(_logFont);
    _ENGINE::LocalReadLock lock(_log_sink->_rw_lock);
    const char *buf = _log_sink->_buffer.begin();
    const char *buf_end = _log_sink->_buffer.end();
    if (_filter.IsActive()) {
        for (int line_no = 0; line_no < _log_sink->_line_offset.Size; line_no++) {
            const char *line_start = buf + _log_sink->_line_offset[line_no];
            const char *line_end = (line_no + 1 < _log_sink->_line_offset.Size) ?
                                   (buf + _log_sink->_line_offset[line_no + 1] - 1) : buf_end;
            if (_filter.PassFilter(line_start, line_end))
                ImGui::TextUnformatted(line_start, line_end);
        }
    } else {
        ImGuiListClipper clipper;
        clipper.Begin(_log_sink->_line_offset.Size);
        while (clipper.Step()) {
            for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++) {
                const char *line_start = buf + _log_sink->_line_offset[line_no];
                const char *line_end = (line_no + 1 < _log_sink->_line_offset.Size) ? (buf +
                                                                                       _log_sink->_line_offset[line_no +
                                                                                                               1] - 1)
                                                                                    : buf_end;
                ImGui::TextUnformatted(line_start, line_end);
            }
        }
        clipper.End();
    }
    ImGui::PopFont();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    if (_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

void LogWindow::Clear() {
    _ENGINE::LocalWriteLock lock(_log_sink->_rw_lock);
    _log_sink->_buffer.clear();
    _log_sink->_line_offset.clear();
    _log_sink->_line_offset.push_back(0);
}

void LogWindow::OnDestroy() {

}
_END_KONAI3D
