#pragma once
#include "bbs_base.h"

typedef enum {
  TOOL_TYPE_BUILD_SYSTEM,
  TOOL_TYPE_C_COMPILER,
  TOOL_TYPE_CPP_COMPILER,
  TOOL_TYPE_ARCHIVERS,
  TOOL_TYPE_LINKERS,
  TOOL_TYPE_MISC,
} tool_type;

typedef struct {
  const char* id;
  tool_type type;
  const char* path;
  const char* version;
} tool;

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
  tool_type type;
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
  const char* probe_native_arch;
  bool probe_has_native_gcc;
  bool probe_has_native_gpp;
  bool probe_has_x86_64_gcc;
  bool probe_has_x86_64_gpp;
  bool probe_has_x86_c_multilib;
  bool probe_has_x86_cpp_multilib;
  bool probe_has_arm64_gcc;
  bool probe_has_arm64_gpp;
  bool probe_has_arm64_c_cross;
  bool probe_has_arm64_cpp_cross;
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

  toolchain_env* envs;
  int env_c;
  int env_cap;
} toolchain;

static const char* toolchain_get_host_tool_path(toolchain* tc, const char* id);
static const char* toolchain_get_bash_path(toolchain* tc);
static int toolchain_run_bash(toolchain* tc, const char* workdir, const char* script);

static const tool_discover_strat TOOL_DISCOVER_STRATS[] = {
    // BUILD_SYSTEM
    {
     .id = "cmake",
     .type = TOOL_TYPE_BUILD_SYSTEM,
     .target_os = OS_MAX,
     .exe_name = "cmake",
     .dir_hints = "{program_files}\\CMake\\bin;{program_files_x86}\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{program_files}\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;{user_profile}\\scoop\\apps\\cmake\\current\\bin;C:\\ProgramData\\chocolatey\\bin",
     .deep_roots = "{program_files};{program_files_x86}",
     .version_arg = "--version",
     .version_regex = "cmake version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "make",
     .type = TOOL_TYPE_BUILD_SYSTEM,
     .target_os = OS_MAX,
     .exe_name = "make",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;{msys2_root}\\mingw32\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;{user_profile}\\scoop\\apps\\make\\current\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "GNU Make ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "ninja",
     .type = TOOL_TYPE_BUILD_SYSTEM,
     .target_os = OS_MAX,
     .exe_name = "ninja",
     .dir_hints = "{program_files}\\Ninja;{local_app_data}\\Microsoft\\WinGet\\Packages;{user_profile}\\scoop\\apps\\ninja\\current;C:\\ProgramData\\chocolatey\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{local_app_data};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "premake5",
     .type = TOOL_TYPE_BUILD_SYSTEM,
     .target_os = OS_MAX,
     .exe_name = "premake5",
     .dir_hints = "{program_files}\\Premake;{user_profile}\\tools\\premake;/usr/local/bin",
     .deep_roots = "{program_files};{user_profile};/usr/local",
     .version_arg = "--version",
     .version_regex = "premake5? ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },

    // C_COMPILER
    {
     .id = "gcc",
     .type = TOOL_TYPE_C_COMPILER,
     .target_os = OS_MAX,
     .exe_name = "gcc",
     .dir_hints = "{msys2_root}\\mingw64\\bin;{msys2_root}\\ucrt64\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;C:\\mingw64\\bin;C:\\TDM-GCC-64\\bin;C:\\Strawberry\\c\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "gcc( \\(.*\\))? ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     .version_arg_fallback = "-dumpfullversion",
     .version_regex_fallback = "([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "clang",
     .type = TOOL_TYPE_C_COMPILER,
     .target_os = OS_MAX,
     .exe_name = "clang",
     .dir_hints = "{llvm_root}\\bin;{program_files}\\LLVM\\bin;{program_files_x86}\\LLVM\\bin;{user_profile}\\scoop\\apps\\llvm\\current\\bin;C:\\Program Files\\LLVM\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/opt/llvm/bin;/opt/homebrew/bin",
     .deep_roots = "{llvm_root};{program_files};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "clang version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    // CPP_COMPILER
    {
     .id = "g++",
     .type = TOOL_TYPE_CPP_COMPILER,
     .target_os = OS_MAX,
     .exe_name = "g++",
     .dir_hints = "{msys2_root}\\mingw64\\bin;{msys2_root}\\ucrt64\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;C:\\mingw64\\bin;C:\\TDM-GCC-64\\bin;C:\\Strawberry\\c\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "g\\+\\+( \\(.*\\))? ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "clang++",
     .type = TOOL_TYPE_CPP_COMPILER,
     .target_os = OS_MAX,
     .exe_name = "clang++",
     .dir_hints = "{llvm_root}\\bin;{program_files}\\LLVM\\bin;{program_files_x86}\\LLVM\\bin;{user_profile}\\scoop\\apps\\llvm\\current\\bin;C:\\Program Files\\LLVM\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/opt/llvm/bin;/opt/homebrew/bin",
     .deep_roots = "{llvm_root};{program_files};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "clang version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    // ARCHIVERS
    {
     .id = "ar",
     .type = TOOL_TYPE_ARCHIVERS,
     .target_os = OS_MAX,
     .exe_name = "ar",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "GNU ar(.*?) ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "llvm-ar",
     .type = TOOL_TYPE_ARCHIVERS,
     .target_os = OS_MAX,
     .exe_name = "llvm-ar",
     .dir_hints = "{llvm_root}\\bin;{program_files}\\LLVM\\bin;{program_files_x86}\\LLVM\\bin;{user_profile}\\scoop\\apps\\llvm\\current\\bin;C:\\Program Files\\LLVM\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/opt/llvm/bin;/opt/homebrew/bin",
     .deep_roots = "{llvm_root};{program_files};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "LLVM version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    // LINKERS
    {
     .id = "ld",
     .type = TOOL_TYPE_LINKERS,
     .target_os = OS_MAX,
     .exe_name = "ld",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "GNU ld(.*?) ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "lld",
     .type = TOOL_TYPE_LINKERS,
     .target_os = OS_MAX,
     .exe_name = "lld",
     .dir_hints = "{llvm_root}\\bin;{program_files}\\LLVM\\bin;{program_files_x86}\\LLVM\\bin;{user_profile}\\scoop\\apps\\llvm\\current\\bin;C:\\Program Files\\LLVM\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/opt/llvm/bin;/opt/homebrew/bin",
     .deep_roots = "{llvm_root};{program_files};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "LLD ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    // MISC
    {
     .id = "objdump",
     .type = TOOL_TYPE_MISC,
     .target_os = OS_MAX,
     .exe_name = "objdump",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "objdump(.*?) ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "nm",
     .type = TOOL_TYPE_MISC,
     .target_os = OS_MAX,
     .exe_name = "nm",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "nm(.*?) ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "strings",
     .type = TOOL_TYPE_MISC,
     .target_os = OS_MAX,
     .exe_name = "strings",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "strings(.*?) ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "docker",
     .type = TOOL_TYPE_MISC,
     .target_os = OS_MAX,
     .exe_name = "docker",
     .dir_hints = "{program_files}\\Docker\\Docker\\resources\\bin;{local_app_data}\\Programs\\Docker\\Docker\\resources\\bin;C:\\Program Files\\Docker\\Docker\\resources\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{local_app_data};/usr;/usr/local;/opt/homebrew",
     .version_arg = "--version",
     .version_regex = "Docker version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "bash",
     .type = TOOL_TYPE_MISC,
     .target_os = OS_MAX,
     .exe_name = "bash",
     .dir_hints = "{program_files}\\Git\\bin;{program_files}\\Git\\usr\\bin;{program_files_x86}\\Git\\bin;{program_files_x86}\\Git\\usr\\bin;{msys2_root}\\usr\\bin;C:\\msys64\\usr\\bin;C:\\Program Files\\Git\\bin;C:\\Program Files\\Git\\usr\\bin;/bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{program_files};{program_files_x86};{msys2_root};/usr;/usr/local;/opt/homebrew",
     .version_arg = "--version",
     .version_regex = "version ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "wsl",
     .type = TOOL_TYPE_MISC,
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
     .id = "readelf",
     .type = TOOL_TYPE_MISC,
     .target_os = OS_MAX,
     .exe_name = "readelf",
     .dir_hints = "{msys2_root}\\usr\\bin;{msys2_root}\\mingw64\\bin;C:\\msys64\\usr\\bin;C:\\msys64\\mingw64\\bin;C:\\msys64\\ucrt64\\bin;/usr/bin;/usr/local/bin;/opt/homebrew/bin",
     .deep_roots = "{msys2_root};/usr;/usr/local",
     .version_arg = "--version",
     .version_regex = "readelf(.*?) ([0-9]+\\.[0-9]+(\\.[0-9]+)?)",
     },
    {
     .id = "vcvarsall",
     .type = TOOL_TYPE_MISC,
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
