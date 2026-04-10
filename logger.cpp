#include "logger.h"
#include <iomanip>

Logger g_logger;

Logger::Logger() {}
Logger::~Logger() { close(); }

void Logger::open(const std::string& filename) {
    file_.open(filename.c_str(), std::ios::out);
}

void Logger::close() {
    if (file_.is_open()) file_.close();
}

bool Logger::is_open() const {
    return file_.is_open();
}

void Logger::flush() {
    file_.flush();
}

std::string Logger::getTimestamp() const {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    std::ostringstream oss;
    oss << tv.tv_sec << "." << std::setfill('0') << std::setw(6) << tv.tv_usec;
    return oss.str();
}

std::string Logger::levelToString(Level level) const {
    switch (level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO ";
        case WARNING: return "WARN ";
        case ERROR:   return "ERROR";
        default:      return "INFO ";
    }
}

void Logger::log(Level level, const std::string& msg) {
    if (!file_.is_open()) return;
    file_ << "[" << getTimestamp() << "] [" << levelToString(level) << "] " << msg << "\n";
}

void Logger::debug(const std::string& msg)   { log(DEBUG,   msg); }
void Logger::info(const std::string& msg)    { log(INFO,    msg); }
void Logger::warning(const std::string& msg) { log(WARNING, msg); }
void Logger::error(const std::string& msg)   { log(ERROR,   msg); }
