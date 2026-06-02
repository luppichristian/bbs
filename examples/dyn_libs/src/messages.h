#pragma once

#if defined(_WIN32)
#  if defined(MESSAGES_EXPORTS)
#    define MESSAGES_API __declspec(dllexport)
#  else
#    define MESSAGES_API __declspec(dllimport)
#  endif
#else
#  define MESSAGES_API
#endif

MESSAGES_API const char* messages_prefix(void);
