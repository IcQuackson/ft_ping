#include "logger.h"

void log_message(int level, const char *message, ...) {
	va_list args;
	va_start(args, message);
	char *level_str = NULL;

	if (level >= LOG_LEVEL) {
		switch (level) {
			case DEBUG:
				level_str = "DEBUG";
				printf(DEBUG_COLOR);
				break;
			case INFO:
				level_str = "INFO";
				printf(INFO_COLOR);
				break;
			case WARN:
				level_str = "WARN";
				printf(WARN_COLOR);
				break;
			case ERROR:
				level_str = "ERROR";
				printf(ERROR_COLOR);
				break;
			default:
				level_str = "UNKNOWN";
				break;
		}

		printf("[%s] ", level_str);
		vprintf(message, args);
		printf("\n");
		va_end(args);
		printf("\033[0m");
	}

	va_end(args);

}