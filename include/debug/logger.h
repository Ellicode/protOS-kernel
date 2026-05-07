#ifndef LOGGER_H
#define LOGGER_H

void k_error(const char *message, const char *scope);
void k_info(const char *message, const char *scope);
void k_success(const char *message, const char *scope);
void k_warning(const char *message, const char *scope);
void k_debug(const char *message, const char *scope);

#endif // LOGGER_H