#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer
#include "Common/Include/Renderer.h"
#include "Common/Include/common.h"

#include "jniapi.h"
#include "android_fopen.h"

static ANativeWindow *window = 0;
static Renderer *renderer = 0;
static JavaVM *g_vm = 0;
static jobject g_jobj;

JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnStart(JNIEnv* jenv, jobject obj)
{
    renderer = new Renderer();
}

JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnResume(JNIEnv* jenv, jobject obj)
{
    if (renderer) renderer->start();
}

JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnPause(JNIEnv* jenv, jobject obj)
{
    if (renderer) renderer->stop();
}

JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeOnStop(JNIEnv* jenv, jobject obj)
{
    if (renderer) delete renderer;
    renderer = 0;
}


JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeSetSurface(JNIEnv* jenv, jobject obj, jobject surface)
{
    if (surface != 0) {
        renderer->start();

        window = ANativeWindow_fromSurface(jenv, surface);
        renderer->setWindow(window);

        jenv->GetJavaVM(&g_vm);
        g_jobj = jenv->NewGlobalRef(obj);
    } else {
        ANativeWindow_release(window);
        renderer->stop();

        jenv->DeleteGlobalRef(g_jobj);
    }
}

JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeSetBench(JNIEnv* jenv, jobject obj, jint type)
{
    renderer->setBench((Renderer::BENCH_TYPE)type);
}

JNIEXPORT void JNICALL Java_com_example_simple_MainActivity_nativeSetResPath(JNIEnv* jenv, jobject obj, jstring path)
{
    android_fopen_set_res_path(jenv->GetStringUTFChars(path, JNI_FALSE));
}

JNIEXPORT jfloat JNICALL Java_com_example_simple_MainActivity_nativeGetFps(JNIEnv* jenv, jobject obj, jint interval)
{
    return renderer->getCurFps(interval);
}

void setDone()
{
    JNIEnv *jenv = 0;
    g_vm->GetEnv((void**)&jenv, JNI_VERSION_1_6);
    g_vm->AttachCurrentThread(&jenv, NULL);

    jclass clazz = jenv->GetObjectClass(g_jobj);
    jmethodID setDoneCallback = jenv->GetMethodID(clazz, "setDone", "()V");
    jenv->CallVoidMethod(g_jobj, setDoneCallback);
    g_vm->DetachCurrentThread();
}

void setBenchResult(float fps, std::string nextTitle)
{
    JNIEnv *jenv = 0;
    g_vm->GetEnv((void**)&jenv, JNI_VERSION_1_6);
    g_vm->AttachCurrentThread(&jenv, NULL);

    jclass clazz = jenv->GetObjectClass(g_jobj);
    jmethodID setBenchResultCallback = jenv->GetMethodID(clazz, "setBenchResult", "(FLjava/lang/String;)V");
    jenv->CallVoidMethod(g_jobj, setBenchResultCallback, fps, jenv->NewStringUTF(nextTitle.c_str()));
    g_vm->DetachCurrentThread();
}

void setBenchTitle(std::string title)
{
    JNIEnv *jenv = 0;
    g_vm->GetEnv((void**)&jenv, JNI_VERSION_1_6);
    g_vm->AttachCurrentThread(&jenv, NULL);

    jclass clazz = jenv->GetObjectClass(g_jobj);
    jmethodID setBenchTitleCallback = jenv->GetMethodID(clazz, "setBenchTitle", "(Ljava/lang/String;)V");
    jenv->CallVoidMethod(g_jobj, setBenchTitleCallback, jenv->NewStringUTF(title.c_str()));
    g_vm->DetachCurrentThread();
}
