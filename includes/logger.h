#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>

#define DEBUG 1
#define INFO 2
#define WARN 3
#define ERROR 4
#define NO_LOG 5

#define DEBUG_COLOR "\033[0;36m"
#define INFO_COLOR "\033[0;32m"
#define WARN_COLOR "\033[0;33m"
#define ERROR_COLOR "\033[0;31m"

#define LOG_LEVEL INFO

void log_message(int level, const char *message, ...);

#endif