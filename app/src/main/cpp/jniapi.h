#include <jni.h>

#ifndef JNIAPI_H
#define JNIAPI_H

extern "C" {
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnStart(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnResume(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnPause(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnStop(JNIEnv* jenv, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface);
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeSetBench(JNIEnv* jenv, jobject obj, jint type);
    JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeSetResPath(JNIEnv* jenv, jobject obj, jstring path);
    JNIEXPORT jfloat JNICALL Java_com_example_simple_MainActivity_nativeGetFps(JNIEnv* jenv, jobject obj, jint interval);
};

void setDone();
void setBenchResult(float fps, std::string nextTitle);
void setBenchTitle(std::string title);

#endif // JNIAPI_H
