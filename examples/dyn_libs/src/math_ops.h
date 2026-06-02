#pragma once

#if defined(_WIN32)
#  if defined(MATH_OPS_EXPORTS)
#    define MATH_OPS_API __declspec(dllexport)
#  else
#    define MATH_OPS_API __declspec(dllimport)
#  endif
#else
#  define MATH_OPS_API
#endif

MATH_OPS_API int math_ops_add(int a, int b);
MATH_OPS_API const char* math_ops_summary(void);
