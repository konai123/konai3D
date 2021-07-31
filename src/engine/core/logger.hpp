//
// Created by korona on 2021-06-08.
//

#ifndef KONAI3D_LOGGER_HPP
#define KONAI3D_LOGGER_HPP

_START_ENGINE
class Logger {
public:
    static spdlog::logger* Instance(std::string logFilePath) {
        if (_spd_loggers.count(logFilePath) == 0) {
            spdlog::set_pattern("%^[%T] %n: %v%$");
            auto max_size = 1048576 * 5;
            auto max_files = 3;
            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
            sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logFilePath, max_size, max_files));
            spdlog::flush_every (std::chrono::seconds(5));
            _spd_loggers[logFilePath] = std::make_shared<spdlog::logger>(logFilePath, begin(sinks), end(sinks));
            spdlog::register_logger(_spd_loggers[logFilePath]);
        }
        return _spd_loggers[logFilePath].get();
    };

    template<typename Mutex>
    static void AddSink(std::shared_ptr<spdlog::sinks::base_sink<Mutex>> sink, std::string logFilePath) {
        Instance(logFilePath)->sinks().push_back(sink);
    }

private:
    inline static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> _spd_loggers;

private:
    Logger() = delete;
};
_END_ENGINE

#endif //KONAI3D_LOGGER_HPP
