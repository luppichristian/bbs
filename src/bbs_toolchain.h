#pragma once
#include "bbs_base.h"
#include "../pub/bbs/build.h"

typedef bbs_tool tool;
typedef bbs_toolchain_attr_kind toolchain_attr_kind;
typedef bbs_toolchain_attr_info toolchain_attr_info;
typedef bbs_sdk sdk;
typedef bbs_arch arch;
typedef bbs_os os;
typedef bbs_tool_discover_strat tool_discover_strat;
typedef bbs_sdk_discover_strat sdk_discover_strat;
typedef bbs_toolchain_env toolchain_env;
typedef bbs_toolchain toolchain;

#define p_arch host_arch
#define p_os host_os

#define TOOLCHAIN_ATTR_KIND_STRING BBS_TOOLCHAIN_ATTR_STRING
#define TOOLCHAIN_ATTR_KIND_IDENTIFIER BBS_TOOLCHAIN_ATTR_IDENTIFIER
#define TOOLCHAIN_ATTR_KIND_SECTION BBS_TOOLCHAIN_ATTR_SECTION

#define ARCH_X86_64 BBS_ARCH_X86_64
#define ARCH_X86 BBS_ARCH_X86
#define ARCH_ARM64 BBS_ARCH_ARM64
#define ARCH_MAX BBS_ARCH_MAX

#define OS_WINDOWS BBS_OS_WINDOWS
#define OS_LINUX BBS_OS_LINUX
#define OS_MACOS BBS_OS_MACOS
#define OS_MAX BBS_OS_MAX

#define ARCH_NAMES BBS_ARCH_NAMES
#define OS_NAMES BBS_OS_NAMES

#define TOOLCHAIN_HOST_ATTR_INFOS BBS_TOOLCHAIN_HOST_ATTR_INFOS
#define TOOLCHAIN_TOOL_ATTR_INFOS BBS_TOOLCHAIN_TOOL_ATTR_INFOS
#define TOOLCHAIN_SDK_ATTR_INFOS BBS_TOOLCHAIN_SDK_ATTR_INFOS
#define TOOLCHAIN_PROBE_ATTR_INFOS BBS_TOOLCHAIN_PROBE_ATTR_INFOS
#define TOOLCHAIN_ENV_ATTR_INFOS BBS_TOOLCHAIN_ENV_ATTR_INFOS
#define TOOLCHAIN_ROOT_ATTR_INFOS BBS_TOOLCHAIN_ROOT_ATTR_INFOS

#define TOOLCHAIN_MSVC_SUPPORTED_ARCHES BBS_TOOLCHAIN_MSVC_SUPPORTED_ARCHES
#define TOOLCHAIN_XCODE_SUPPORTED_ARCHES BBS_TOOLCHAIN_XCODE_SUPPORTED_ARCHES

#define TOOL_DISCOVER_STRATS BBS_TOOL_DISCOVER_STRATS
#define SDK_DISCOVER_STRATS BBS_SDK_DISCOVER_STRATS
