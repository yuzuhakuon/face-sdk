#pragma once
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <type_traits>
#ifdef __ANDROID__
#include <spdlog/sinks/android_sink.h>
#endif

#include "gawrs_face/utilities/singleton.hpp"

#define LOG_PREFIX "gawrs_face"

class Logger : public Singleton<Logger>
{
    friend class Singleton<Logger>;
#ifdef __ANDROID__
    using console_sink_mt = spdlog::sinks::android_sink_mt;
#else
    using console_sink_mt = spdlog::sinks::stdout_color_sink_mt;
#endif

    // using file_sink_mt = spdlog::sinks::daily_file_sink_mt;
    using file_sink_mt = spdlog::sinks::rotating_file_sink_mt;

public:
    auto& logger() const { return this->logger_; }

private:
    Logger()
    {
        initFile();

        initConsole();

        initLogger();
    }

private:
    void initFile()
    {
        int param1, param2;
        if constexpr (std::is_same_v<file_sink_mt, spdlog::sinks::rotating_file_sink_mt>)
        {
            int maxSize = 1024 * 1024 * 1; // Mb
            int maxFiles = 3;
            param1 = maxSize;
            param2 = maxFiles;
        }
        else
        {
            int rotationH = 5;
            int rotationM = 59;
            param1 = rotationH;
            param2 = rotationM;
        }

        fileSink_ = std::make_shared<file_sink_mt>(logRootPath_ + infoFilePath_, param1, param2);
        sinks_.push_back(fileSink_);

        errorSink_ = std::make_shared<file_sink_mt>(logRootPath_ + errorFilePath_, param1, param2);
        errorSink_->set_level(spdlog::level::warn);
        sinks_.push_back(errorSink_);
    }

    void initConsole()
    {
        consoleSink_ = std::make_shared<console_sink_mt>();
        sinks_.push_back(consoleSink_);
    }

    void initLogger()
    {
        logger_ = std::make_shared<spdlog::logger>(LOG_PREFIX, sinks_.begin(), sinks_.end());
        logger_->set_pattern("[%l] [%Y-%m-%d %H:%M:%S,%e,%P] - %v");
#ifdef _DEBUG
        logger_->set_level(spdlog::level::trace);
#endif
        logger_->flush_on(spdlog::level::warn);
        spdlog::register_logger(logger_);              // 注册logger
        spdlog::flush_every(std::chrono::seconds(30)); // 每隔30秒刷新一次日志
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
    std::vector<spdlog::sink_ptr> sinks_;

    std::shared_ptr<file_sink_mt> fileSink_;       // file
    std::shared_ptr<file_sink_mt> errorSink_;      // error
    std::shared_ptr<console_sink_mt> consoleSink_; // console

#ifdef __ANDROID__
    std::string logRootPath_{"/sdcard/" LOG_PREFIX "/logs/"};
#else
    std::string logRootPath_{"./logs/"};
#endif
    std::string infoFilePath_{LOG_PREFIX ".runtime.log"};
    std::string errorFilePath_{LOG_PREFIX ".error.log"};
};
