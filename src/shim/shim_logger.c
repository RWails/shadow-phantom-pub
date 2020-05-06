#include "shim/shim_logger.h"

#include <stdio.h>

#include "support/logger/logger.h"
#include "support/logger/log_level.h"

typedef struct _ShimLogger {
    Logger base;
    FILE* file;
} ShimLogger;

void shimlogger_log(Logger* base, LogLevel level, const gchar* fileName,
                    const gchar* functionName, const gint lineNumber,
                    const gchar* format, va_list vargs) {
    ShimLogger* logger = (ShimLogger*)base;

    gchar* message = g_strdup_vprintf(format, vargs);
    fprintf(logger->file, "[shd-shim] [%s] [%s:%i] [%s] %s\n",
            loglevel_toStr(level), fileName, lineNumber, functionName, message);
    g_free(message);
}

void shimlogger_destroy(Logger* logger) {
    free(logger);
}

Logger* shimlogger_new(FILE* file) {
    ShimLogger* logger = malloc(sizeof(*logger));
    *logger = (ShimLogger){
        .base =
            {
                .log = shimlogger_log,
                .destroy = shimlogger_destroy,
            },
        .file = file,
    };
    return (Logger*)logger;
}