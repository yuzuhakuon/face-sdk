#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <jni.h>
#include <memory>
#include <type_traits>
#include <vector>

class FaceFeatureManaged
{
public:
    FaceFeatureManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);

        constructor = env->GetMethodID(cls, "<init>", "()V");

        featureData = env->GetFieldID(cls, "featureData", "[B");
    }

public:
    static constexpr char kSignature[] = "com/gawrs/face/FaceFeature";

    // Class
    jclass cls;

    // Method
    jmethodID constructor;

    // Field
    jfieldID featureData;
};

class JFaceFeatureRef
{
public:
    JFaceFeatureRef(JNIEnv* env, std::shared_ptr<FaceFeatureManaged> managed, jobject jobj)
        : env_(env), managed_(managed), thiz(jobj)
    {
        arr_ = (jbyteArray)env_->GetObjectField(thiz, managed_->featureData);
        len_ = env->GetArrayLength(arr_);
     }

public:
    void copyTo(jbyte* buf, int bufSize) const { env_->GetByteArrayRegion(arr_, 0, bufSize, buf); }
    void copyTo(jbyte* buf) const { env_->GetByteArrayRegion(arr_, 0, len_, buf); }

    void setData(jbyte* byteData, int length) { env_->SetByteArrayRegion(arr_, 0, length, byteData); }

    int size() const { return len_; }

    jobject self() { return thiz; }

private:
    JNIEnv* env_;
    std::shared_ptr<FaceFeatureManaged> managed_;

    jobject thiz;
    jbyteArray arr_;
    int len_;
};
