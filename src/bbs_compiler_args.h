#pragma once

static const char* compiler_args_translate_msvc(const char* text, bool is_cpp, bool* out_changed);
static const char* compiler_args_translate_nvcc(const char* text, bool host_is_msvc, bool* out_changed);
