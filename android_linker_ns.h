// SPDX-License-Identifier: BSD-2-Clause
// Copyright © 2021 Billy Laws

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <android/dlext.h>
#include <stdint.h>

// https://cs.android.com/android/platform/superproject/+/0a492a4685377d41fef2b12e9af4ebfa6feef9c2:art/libnativeloader/include/nativeloader/dlext_namespaces.h;l=25;bpv=1;bpt=1
enum {
  ANDROID_NAMESPACE_TYPE_REGULAR = 0,
  ANDROID_NAMESPACE_TYPE_ISOLATED = 1,
  ANDROID_NAMESPACE_TYPE_SHARED = 2,
  ANDROID_NAMESPACE_TYPE_EXEMPT_LIST_ENABLED = 0x08000000,
  ANDROID_NAMESPACE_TYPE_ALSO_USED_AS_ANONYMOUS = 0x10000000,
  ANDROID_NAMESPACE_TYPE_SHARED_ISOLATED = ANDROID_NAMESPACE_TYPE_SHARED | ANDROID_NAMESPACE_TYPE_ISOLATED,
};

/**
 * @brief Checks if linkernsbypass loaded successfully and is safe to use
 * @note IMPORTANT: This should be called before any calls to the rest of the library are made
 * @return true if loading succeeded
 */
bool linkernsbypass_load_status();

// https://cs.android.com/android/platform/superproject/+/0a492a4685377d41fef2b12e9af4ebfa6feef9c2:art/libnativeloader/include/nativeloader/dlext_namespaces.h;l=86;bpv=1;bpt=1
struct android_namespace_t *android_create_namespace(const char *name,
                                                     const char *ld_library_path,
                                                     const char *default_library_path,
                                                     uint64_t type,
                                                     const char *permitted_when_isolated_path,
                                                     struct android_namespace_t *parent_namespace);

struct android_namespace_t *android_create_namespace_escape(const char *name,
                                                            const char *ld_library_path,
                                                            const char *default_library_path,
                                                            uint64_t type,
                                                            const char *permitted_when_isolated_path,
                                                            struct android_namespace_t *parent_namespace);

// https://cs.android.com/android/platform/superproject/+/dcb01ef31026b3b8aeb72dada3370af63fe66bbd:bionic/linker/linker.cpp;l=3554
typedef struct android_namespace_t *(*android_get_exported_namespace_t)(const char *);
extern android_get_exported_namespace_t android_get_exported_namespace;

// https://cs.android.com/android/platform/superproject/+/dcb01ef31026b3b8aeb72dada3370af63fe66bbd:bionic/linker/linker.cpp;l=2499
typedef bool (*android_link_namespaces_all_libs_t)(struct android_namespace_t *, struct android_namespace_t *);
extern android_link_namespaces_all_libs_t android_link_namespaces_all_libs;

// https://cs.android.com/android/platform/superproject/+/dcb01ef31026b3b8aeb72dada3370af63fe66bbd:bionic/linker/linker.cpp;l=2473
typedef bool (*android_link_namespaces_t)(struct android_namespace_t *, struct android_namespace_t *, const char *);
extern android_link_namespaces_t android_link_namespaces;

// android_dlopen_ext but loaded directly from libdl hence ignoring any hooks
extern decltype(&android_dlopen_ext) libdl_android_dlopen_ext;

/**
 * @brief Effectively LD_PRELOAD for a specific namespace with extra bells and whistles
 * @note  IMPORTANT: hook libraries supplied to this must be compiled with the '-z global' linker flag
 * @param hookLibName The soname of the library with hooks to be loaded into `hookNs`
 * @param hookNs The namespace to load the hook into
 * @param hookParam will be written to the `hook_param` symbol in the hook library straight after loading if it is not null
 */
bool linkernsbypass_namespace_apply_hook(const char *hookLibName, struct android_namespace_t *hookNs, const void *hookParam);

/**
 * @brief Effectively a library isolated LD_PRELOAD + dlopen combo with extra bells and whistles
 * @note  IMPORTANT: hook libraries supplied to this must be compiled with the '-z global' linker flag
 * @param libPath The path to the library to load with hooks applied
 * @param libTargetDir A temporary directory to hold the soname patched library at `libPath`, will attempt to use memfd if nullptr
 * @param mode The rtld open mode for `libName`
 * @param hookLibDir The path to the directory containing hooks to be hooks applied
 * @param hookLibName The soname of the library with hooks to be to be applied to `libName`, if nullptr no hooks will be applied
 * @param parentNs Controls the parent symbol namespace of the isolated hook namespace, if nullptr is passed the default namespace will be used
 * @param linkToDefault If this new namespace should be linked to the default namespace
 * @param hookParam will be written to the `hook_param` symbol in the hook library straight after loading if it is not null
 */
void *linkernsbypass_dlopen_unique_hooked(const char *libPath, const char *libTargetDir, int mode, const char *hookLibDir, const char *hookLibName, struct android_namespace_t *parentNs, bool linkToDefault, const void *hookParam);

#ifdef __cplusplus
}
#endif
