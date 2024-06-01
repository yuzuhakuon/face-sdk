#pragma once
#include <cstring>
#include <jni.h>
#include <memory>
#include <vector>

class FaceKitManaged
{
public:
    FaceKitManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);
        mHandle = env->GetFieldID(cls, "handle", "J");
    }

    void setHandle(JNIEnv* env, jobject obj, long handle) { env->SetLongField(obj, mHandle, handle); }

    long handle(JNIEnv* env, jobject obj) { return env->GetLongField(obj, mHandle); }

public:
    static constexpr char kSignature[] = "com/gawrs/face/FaceKit";

    // Class
    jclass cls;

    // Field
    jfieldID mHandle;
};