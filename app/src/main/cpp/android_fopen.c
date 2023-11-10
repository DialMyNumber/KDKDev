#include <stdio.h>
#include "android_fopen.h"

char g_path[256] = "";
void android_fopen_set_res_path(const char* path) {
	sprintf(g_path, "%s", path);
}

const char* android_fopen_get_res_path() {
	return g_path;
}
