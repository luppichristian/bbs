#pragma once
#include "bbs_base.h"

typedef struct {
  const char* id;
  const char* path;
  const char* version;
} tool;

typedef enum {
  TOOLCHAIN_ATTR_KIND_STRING,
  TOOLCHAIN_ATTR_KIND_IDENTIFIER,
  TOOLCHAIN_ATTR_KIND_SECTION,
} toolchain_attr_kind;

typedef struct {
  const char* name;
  toolchain_attr_kind kind;
  bool required;
} toolchain_attr_info;

typedef struct {
  const char* name;     // "windows-sdk", "xcode", "esp-idf", ...
  const char* version;  // optional
  const char* base_path;
  const char* inc_path;
  const char* src_path;
  const char* lib_path;
  const char* bin_path;
} sdk;

typedef enum {
  ARCH_X86_64,
  ARCH_X86,
  ARCH_ARM64,
  ARCH_MAX,
} arch;

static const char* ARCH_NAMES[] = {
    [ARCH_X86_64] = "x86_64",
    [ARCH_X86] = "x86",
    [ARCH_ARM64] = "arm64",
};

typedef enum {
  OS_WINDOWS,
  OS_LINUX,
  OS_MACOS,
  OS_MAX,
} os;

static const char* OS_NAMES[] = {
    [OS_WINDOWS] = "windows",
    [OS_LINUX] = "linux",
    [OS_MACOS] = "macos",
};

typedef struct {
  const char* id;
  os target_os;  // OS_MAX == any

  // Stage 1: executable lookup in PATH/where/which.
  const char* exe_name;

  // Stage 2: targeted directories (optional, ';' separated).
  // Tokens: {home} {program_files} {program_files_x86} {local_app_data}
  //         {user_profile} {xcode_dev_root} {llvm_root} {msys2_root}
  const char* dir_hints;

  // Stage 3: optional deep-search roots (';' separated, same tokens).
  const char* deep_roots;

  // Version probing.
  const char* version_arg;    // e.g. "--version" or "/?"
  const char* version_regex;  // first capture-group is normalized version
  const char* version_arg_fallback;
  const char* version_regex_fallback;
} tool_discover_strat;

typedef struct {
  const char* id;
  os target_os;  // OS_MAX == any

  // Primary SDK root discovery.
  // env_vars is ';' separated env var list, first existing wins.
  // root_hints is ';' separated absolute/pattern hints with tokens.
  const char* env_vars;
  const char* root_hints;

  // Sub-path metadata (relative to base_path unless explicitly absolute).
  const char* include_rel;
  const char* source_rel;
  const char* lib_rel;
  const char* bin_rel;

  // Version metadata discovery.
  // version_file_rel can be one or more ';' separated candidates.
  const char* version_file_rel;
  const char* version_regex;
} sdk_discover_strat;

typedef struct {
  const char* id;
  const char* provider;
  const char* name;
  arch p_arch;
  os p_os;

  // Cached probe data used to rebuild support at runtime.
  const char* probe_docker_buildx_platforms;

  bool supported[OS_MAX][ARCH_MAX];
  const char* support_source[OS_MAX][ARCH_MAX];

  tool* tools;
  int tool_c;
  int tool_cap;

  sdk* sdks;
  int sdk_c;
  int sdk_cap;
} toolchain_env;

typedef struct {
  // Platform
  arch p_arch;
  os p_os;
  bool supported[OS_MAX][ARCH_MAX];
  const char* support_source[OS_MAX][ARCH_MAX];
  node* config_tree;

    const char* project_cfg_path;
    const char* user_cfg_path;
   const char* local_cfg_path;
   const char* toolchain_cfg_path;

  toolchain_env* envs;
  int env_c;
  int env_cap;
} toolchain;

static const toolchain_attr_info TOOLCHAIN_HOST_ATTR_INFOS[] = {
    {.name = "arch", .kind = TOOLCHAIN_ATTR_KIND_IDENTIFIER, .required = false},
    {.name = "os", .kind = TOOLCHAIN_ATTR_KIND_IDENTIFIER, .required = false},
};

static const toolchain_attr_info TOOLCHAIN_TOOL_ATTR_INFOS[] = {
    {.name = "id", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "path", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = true},
    {.name = "version", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
};

static const toolchain_attr_info TOOLCHAIN_SDK_ATTR_INFOS[] = {
    {.name = "name", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "version", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "base_path", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "inc_path", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "src_path", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "lib_path", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
    {.name = "bin_path", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
};

static const toolchain_attr_info TOOLCHAIN_PROBE_ATTR_INFOS[] = {
    {.name = "docker_buildx_platforms", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = false},
};

static const toolchain_attr_info TOOLCHAIN_ENV_ATTR_INFOS[] = {
    {.name = "id", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = true},
    {.name = "provider", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = true},
    {.name = "name", .kind = TOOLCHAIN_ATTR_KIND_STRING, .required = true},
    {.name = "host", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
    {.name = "probes", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
    {.name = "tools", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
    {.name = "sdks", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
};

static const toolchain_attr_info TOOLCHAIN_ROOT_ATTR_INFOS[] = {
    {.name = "host", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
    {.name = "tools", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
    {.name = "sdks", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
    {.name = "environments", .kind = TOOLCHAIN_ATTR_KIND_SECTION, .required = false},
};

static const arch TOOLCHAIN_MSVC_SUPPORTED_ARCHES[] = {ARCH_X86_64, ARCH_X86, ARCH_ARM64};
static const arch TOOLCHAIN_XCODE_SUPPORTED_ARCHES[] = {ARCH_X86_64, ARCH_ARM64};

static const tool_discover_strat TOOL_DISCOVER_STRATS[] = {
    // BUILD_SYSTEM
    {
     .id = "cmake",
     .target_os = OS_MAX,
     .exe_name = "cmake",
     .dir_hints = "{program_files}\\CMake\\bin;{program_files_x86}\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{user_profile}\\scoop\\apps\\cmake\\current\\bin;C:\\ProgramData\\chocolatey\\bin",
     .deep_roots = "{program_files};{program_files_x86}",
     .version_arg = "--version",
     .version_regex = "cmake version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "ctest",
     .target_os = OS_MAX,
     .exe_name = "ctest",
     .dir_hints = "{program_files}\\CMake\\bin;{program_files_x86}\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{user_profile}\\scoop\\apps\\cmake\\current\\bin;C:\\ProgramData\\chocolatey\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{program_files_x86};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "ctest version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },

    {
     .id = "docker",
     .target_os = OS_MAX,
     .exe_name = "docker",
     .dir_hints = "{program_files}\\Docker\\Docker\\resources\\bin;{local_app_data}\\Programs\\Docker\\Docker\\resources\\bin;C:\\Program Files\\Docker\\Docker\\resources\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{local_app_data};/usr;/usr/local;/opt/homebrew",
     .version_arg = "--version",
     .version_regex = "Docker version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "bash",
     .target_os = OS_MAX,
     .exe_name = "bash",
     .dir_hints = "{program_files}\\Git\\bin;{program_files}\\Git\\usr\\bin;{program_files_x86}\\Git\\bin;{program_files_x86}\\Git\\usr\\bin;{msys2_root}\\usr\\bin;C:\\msys64\\usr\\bin;C:\\Program Files\\Git\\bin;C:\\Program Files\\Git\\usr\\bin;/bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{program_files_x86};{msys2_root};/usr;/usr/local;/opt/homebrew",
     .version_arg = "--version",
     .version_regex = "version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "git",
     .target_os = OS_MAX,
     .exe_name = "git",
     .dir_hints = "{program_files}\\Git\\cmd;{program_files}\\Git\\bin;{program_files_x86}\\Git\\cmd;{program_files_x86}\\Git\\bin;{msys2_root}\\usr\\bin;C:\\msys64\\usr\\bin;C:\\Program Files\\Git\\cmd;C:\\Program Files\\Git\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{program_files_x86};{msys2_root};/usr;/usr/local;/opt/homebrew",
     .version_arg = "--version",
     .version_regex = "version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
     {
     .id = "wsl",
     .target_os = OS_WINDOWS,
     .exe_name = "wsl",
     .dir_hints = "C:\\Windows\\System32;{user_profile}\\AppData\\Local\\Microsoft\\WindowsApps",
     .deep_roots = "",
     .version_arg = "--version",
     .version_regex = "WSL version: ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     .version_arg_fallback = "--status",
     .version_regex_fallback = "Default Version: ([0-9]+)",
     },
    {
     .id = "vcvarsall",
     .target_os = OS_WINDOWS,
     .exe_name = "vcvarsall.bat",
     .dir_hints = "{program_files}\\Microsoft Visual Studio;{program_files_x86}\\Microsoft Visual Studio",
     .deep_roots = "{program_files};{program_files_x86}",
     .version_arg = "",
     .version_regex = "",
     },
};

static const sdk_discover_strat SDK_DISCOVER_STRATS[] = {
    {
     .id = "windows_sdk",
     .target_os = OS_WINDOWS,
     .env_vars = "WindowsSdkDir;WindowsSdkVerBinPath",
     .root_hints = "{program_files_x86}\\Windows Kits\\10;{program_files_x86}\\Windows Kits\\11;C:\\Program Files (x86)\\Windows Kits\\10;C:\\Program Files (x86)\\Windows Kits\\11",
     .include_rel = "Include",
     .source_rel = "Source",
     .lib_rel = "Lib",
     .bin_rel = "bin",
     .version_file_rel = "Include\\**\\um\\Windows.h",
     .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)",
     },
    {
     .id = "ucrt_sdk",
     .target_os = OS_WINDOWS,
     .env_vars = "UniversalCRTSdkDir;UCRTVersion",
     .root_hints = "{program_files_x86}\\Windows Kits\\10;C:\\Program Files (x86)\\Windows Kits\\10",
     .include_rel = "Include",
     .source_rel = "Source",
     .lib_rel = "Lib",
     .bin_rel = "bin",
     .version_file_rel = "Include\\**\\ucrt\\corecrt.h",
     .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+)",
     },
    {
     .id = "msvc",
     .target_os = OS_WINDOWS,
     .env_vars = "VCToolsInstallDir;VCINSTALLDIR",
     .root_hints = "{program_files}\\Microsoft Visual Studio\\*\\*\\VC\\Tools\\MSVC\\*;{program_files_x86}\\Microsoft Visual Studio\\*\\*\\VC\\Tools\\MSVC\\*;C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC;C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Tools\\MSVC;C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\MSVC;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC;C:\\BuildTools\\VC\\Tools\\MSVC",
     .include_rel = "include",
     .source_rel = "",
     .lib_rel = "lib",
     .bin_rel = "bin\\Hostx64\\x64",
     .version_file_rel = "include\\yvals_core.h",
     .version_regex = "_MSC_FULL_VER ([0-9]+)",
     },
    {
     .id = "xcode",
     .target_os = OS_MACOS,
     .env_vars = "DEVELOPER_DIR",
     .root_hints = "/Applications/Xcode.app/Contents/Developer;/Library/Developer/CommandLineTools",
     .include_rel = "",
     .source_rel = "",
     .lib_rel = "Toolchains/XcodeDefault.xctoolchain/usr/lib",
     .bin_rel = "Toolchains/XcodeDefault.xctoolchain/usr/bin",
     .version_file_rel = "../version.plist",
     .version_regex = "<key>CFBundleShortVersionString</key>\\s*<string>([^<]+)</string>",
     },
    {
     .id = "vulkan_sdk",
     .target_os = OS_MAX,
     .env_vars = "VULKAN_SDK",
     .root_hints = "{program_files}\\VulkanSDK\\*;{program_files_x86}\\VulkanSDK\\*;{local_app_data}\\VulkanSDK\\*;C:\\VulkanSDK\\*;{home}/VulkanSDK/*",
     .include_rel = "Include",
     .source_rel = "",
     .lib_rel = "Lib",
     .bin_rel = "Bin",
     .version_file_rel = "README.txt;config\\vk_layer_settings.txt",
     .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "android_ndk",
     .target_os = OS_MAX,
     .env_vars = "ANDROID_NDK_ROOT;ANDROID_NDK_HOME;NDK_ROOT",
     .root_hints = "{home}\\AppData\\Local\\Android\\Sdk\\ndk\\*;{home}\\AppData\\Local\\Android\\Sdk\\ndk-bundle;{local_app_data}\\Android\\Sdk\\ndk\\*;{home}/Android/Sdk/ndk/*;{home}/Android/Sdk/ndk-bundle;/opt/android-ndk;/opt/android-sdk/ndk/*;{program_files}/Android/Android Studio/plugins/android-ndk",
     .include_rel = "toolchains/llvm/prebuilt",
     .source_rel = "sources",
     .lib_rel = "toolchains/llvm/prebuilt",
     .bin_rel = "toolchains/llvm/prebuilt",
     .version_file_rel = "source.properties",
     .version_regex = "Pkg.Revision\\s*=\\s*([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "emsdk",
     .target_os = OS_MAX,
     .env_vars = "EMSDK",
     .root_hints = "{home}\\emsdk;{home}/emsdk;{program_files}\\emsdk;C:\\emsdk;/opt/emsdk",
     .include_rel = "upstream/emscripten/system/include",
     .source_rel = "upstream/emscripten",
     .lib_rel = "upstream/emscripten/cache/sysroot/lib",
     .bin_rel = "upstream/emscripten",
     .version_file_rel = ".emscripten;upstream/emscripten/emcc.py",
     .version_regex = "([0-9]+\\.[0-9]+\\.[0-9]+)",
     },
    {
     .id = "musl",
     .target_os = OS_LINUX,
     .env_vars = "MUSL_ROOT",
     .root_hints = "/usr;/usr/local;/opt/musl;/opt/homebrew/opt/musl",
     .include_rel = "include",
     .source_rel = "src",
     .lib_rel = "lib",
     .bin_rel = "bin",
     .version_file_rel = "include/features.h;lib/libc.so",
     .version_regex = "([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "glibc",
     .target_os = OS_LINUX,
     .env_vars = "GLIBC_ROOT",
     .root_hints = "/usr;/usr/local",
     .include_rel = "include",
     .source_rel = "src",
     .lib_rel = "lib;lib64",
     .bin_rel = "bin",
     .version_file_rel = "include/features.h;lib/libc.so.6",
     .version_regex = "([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
};
