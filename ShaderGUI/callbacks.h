#pragma once
#include "imgui.h"

#include <ctype.h>
#include <list>

int text_edit_callback(ImGuiInputTextCallbackData* data);

int text_resize_callback(ImGuiInputTextCallbackData* data);

void glfw_error_callback(int error, const char* description);

int text_completion_callback(ImGuiInputTextCallbackData* data);

static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }

static char* Strdup(const char* s) { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }

void AddLog(const char* fmt, ...) IM_FMTARGS(2);


