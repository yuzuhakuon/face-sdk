#pragma once
#include <cstring>
#include <iostream>
#include <jni.h>
#include <memory>
#include <vector>

class VersionInfoManaged
{
public:
    VersionInfoManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);
        constructor = env->GetMethodID(cls, "<init>", "()V");
        version = env->GetFieldID(cls, "version", "Ljava/lang/String;");
        buildDate = env->GetFieldID(cls, "buildDate", "Ljava/lang/String;");
        copyRight = env->GetFieldID(cls, "copyRight", "Ljava/lang/String;");
    }

    bool ok() const { return (version != nullptr) && (buildDate != nullptr) && (copyRight != nullptr); }

public:
    static constexpr char kSignature[] = "com/gawrs/face/VersionInfo";

    // Class
    jclass cls;
    // Method
    jmethodID constructor;

    // Field
    jfieldID version;
    jfieldID buildDate;
    jfieldID copyRight;
};

class JVersionInfoRef
{
public:
    JVersionInfoRef(JNIEnv* env, std::shared_ptr<VersionInfoManaged> managed) : env_(env), managed_(managed)
    {
        thiz = env->NewObject(managed->cls, managed->constructor);
    }

    JVersionInfoRef(JNIEnv* env, std::shared_ptr<VersionInfoManaged> managed, jobject jobj)
        : env_(env), managed_(managed), thiz(jobj)
    {
    }

    void setVersion(std::string val) { env_->SetObjectField(thiz, managed_->version, env_->NewStringUTF(val.c_str())); }
    void setBuildDate(std::string val)
    {
        env_->SetObjectField(thiz, managed_->buildDate, env_->NewStringUTF(val.c_str()));
    }
    void setCopyRight(std::string val)
    {
        env_->SetObjectField(thiz, managed_->copyRight, env_->NewStringUTF(val.c_str()));
    }

private:
    JNIEnv* env_;
    std::shared_ptr<VersionInfoManaged> managed_;

    jobject thiz;
};
