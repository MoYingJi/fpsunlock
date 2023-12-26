#pragma once

#include <wchar.h>

void msg_err_a(const char *format, ...);
void msg_err_w(const wchar_t *format, ...);

void msg_warn_a(const char *format, ...);
void msg_warn_w(const wchar_t *format, ...);

void msg_info_a(const char *format, ...);
void msg_info_w(const wchar_t *format, ...);
