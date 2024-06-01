#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <jni.h>
#include <memory>
#include <vector>

class FaceSimilarManaged
{
public:
    FaceSimilarManaged(JNIEnv* env)
    {
        auto cls = env->FindClass(kSignature);

        constructor = env->GetMethodID(cls, "<init>", "()V");
        score = env->GetFieldID(cls, "score", "F");
        databaseId = env->GetFieldID(cls, "databaseId", "I");
        similarId = env->GetFieldID(cls, "similarId", "I");
    }

public:
    static constexpr char kSignature[] = "com/gawrs/face/FaceSimilar";

    // Class
    jclass cls;
    // Method
    jmethodID constructor;

    // Field
    jfieldID score;
    jfieldID databaseId;
    jfieldID similarId;
};

class JFaceSimilarRef
{
public:
    JFaceSimilarRef(JNIEnv* env, std::shared_ptr<FaceSimilarManaged> managed, jobject jobj)
        : env_(env), managed_(managed), thiz(jobj)
    {
    }

    JFaceSimilarRef(JNIEnv* env, std::shared_ptr<FaceSimilarManaged> managed) : env_(env), managed_(managed)
    {
        thiz = env->NewObject(managed->cls, managed->constructor);
    }

public:
    float score() const { return env_->GetFloatField(thiz, managed_->score); }
    int databaseId() const { return env_->GetIntField(thiz, managed_->databaseId); }
    int similarId() const { return env_->GetIntField(thiz, managed_->similarId); }

    void setScore(float val) { env_->SetFloatField(thiz, managed_->score, val); }
    void setDatabaseId(int val) { env_->SetIntField(thiz, managed_->databaseId, val); }
    void setSimilarId(int val) { env_->SetIntField(thiz, managed_->similarId, val); }

    jobject self() { return thiz; }

private:
    JNIEnv* env_;
    std::shared_ptr<FaceSimilarManaged> managed_;

    jobject thiz;
};
