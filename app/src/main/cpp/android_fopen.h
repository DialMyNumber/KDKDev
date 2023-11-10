#ifndef ANDROID_FOPEN_H
#define ANDROID_FOPEN_H

#ifdef __cplusplus
extern "C" {
#endif

void android_fopen_set_res_path(const char* path);
const char* android_fopen_get_res_path();

#ifdef __cplusplus
}
#endif

#endif
