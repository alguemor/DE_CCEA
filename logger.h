#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <sstream>
#include <sys/time.h>

class Logger {
public:
    enum Level { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

    Logger();
    ~Logger();

    void open(const std::string& filename);
    void close();
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);
    void flush();
    bool is_open() const;

private:
    std::ofstream file_;
    void log(Level level, const std::string& msg);
    std::string levelToString(Level level) const;
    std::string getTimestamp() const;
};

extern Logger g_logger;

#define LOG_DEBUG(msg)   do { std::ostringstream _oss; _oss << msg; g_logger.debug(_oss.str());   } while(0)
#define LOG_INFO(msg)    do { std::ostringstream _oss; _oss << msg; g_logger.info(_oss.str());    } while(0)
#define LOG_WARNING(msg) do { std::ostringstream _oss; _oss << msg; g_logger.warning(_oss.str()); } while(0)
#define LOG_ERROR(msg)   do { std::ostringstream _oss; _oss << msg; g_logger.error(_oss.str());   } while(0)

#endif // LOGGER_H
