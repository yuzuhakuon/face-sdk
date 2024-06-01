#pragma once
#include <cstring>
#include <jni.h>
#include <memory>
#include <tuple>
#include <vector>

#include "gawrs_face/types/detection.h"
#include "jni_list.h"

class FaceInfoManaged
{
public:
    FaceInfoManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);

        constructor = env->GetMethodID(cls, "<init>", "()V");

        rect = env->GetFieldID(cls, "rect", "Lcom/gawrs/face/Rectf;");
        keypoints = env->GetFieldID(cls, "keypoints", "Ljava/util/ArrayList;");
        faceId = env->GetFieldID(cls, "faceId", "I");
    }

public:
    static constexpr char kSignature[] = "com/gawrs/face/FaceInfo";

    // Class
    jclass cls;

    // Method
    jmethodID constructor;

    // Field
    jfieldID rect;
    jfieldID keypoints;
    jfieldID faceId;
};

class FaceRectManaged
{
public:
    FaceRectManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);

        constructor = env->GetMethodID(cls, "<init>", "()V");

        left = env->GetFieldID(cls, "left", "F");
        top = env->GetFieldID(cls, "top", "F");
        right = env->GetFieldID(cls, "right", "F");
        bottom = env->GetFieldID(cls, "bottom", "F");
    }

public:
    static constexpr char kSignature[] = "com/gawrs/face/Rectf";

    // Class
    jclass cls;

    // Method
    jmethodID constructor;

    // Field
    jfieldID left;
    jfieldID top;
    jfieldID right;
    jfieldID bottom;
};

class FacePointManaged
{
public:
    FacePointManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);

        constructor = env->GetMethodID(cls, "<init>", "()V");

        x = env->GetFieldID(cls, "x", "F");
        y = env->GetFieldID(cls, "y", "F");
    }

public:
    static constexpr char kSignature[] = "com/gawrs/face/Pointf";

    // Class
    jclass cls;

    // Method
    jmethodID constructor;

    // Field
    jfieldID x;
    jfieldID y;
};

class JFaceInfoRef
{
public:
    JFaceInfoRef(JNIEnv* env,                                        //
                 std::shared_ptr<FaceInfoManaged> faceInfoManaged,   //
                 std::shared_ptr<FaceRectManaged> faceRectManaged,   //
                 std::shared_ptr<FacePointManaged> facePointManaged, //
                 std::shared_ptr<ArrayListManaged> arrayListManaged)
        : env_(env), faceInfoManaged_(faceInfoManaged), faceRectManaged_(faceRectManaged),
          facePointManaged_(facePointManaged), arrayListManaged_(arrayListManaged)
    {
        faceInfo_ = env->NewObject(faceInfoManaged_->cls, faceInfoManaged_->constructor);
        faceRect_ = env->NewObject(faceRectManaged_->cls, faceRectManaged_->constructor);
        facePoints_ = env->NewObject(arrayListManaged_->cls, arrayListManaged_->constructor);

        env->SetObjectField(faceInfo_, faceInfoManaged->rect, faceRect_);
        env->SetObjectField(faceInfo_, faceInfoManaged->keypoints, facePoints_);
    }

    JFaceInfoRef(JNIEnv* env,                                        //
                 std::shared_ptr<FaceInfoManaged> faceInfoManaged,   //
                 std::shared_ptr<FaceRectManaged> faceRectManaged,   //
                 std::shared_ptr<FacePointManaged> facePointManaged, //
                 std::shared_ptr<ArrayListManaged> arrayListManaged, jobject faceInfo)
        : env_(env), faceInfoManaged_(faceInfoManaged), faceRectManaged_(faceRectManaged),
          facePointManaged_(facePointManaged), arrayListManaged_(arrayListManaged), faceInfo_(faceInfo)
    {
        faceRect_ = env->GetObjectField(faceInfo_, faceInfoManaged->rect);
        facePoints_ = env->GetObjectField(faceInfo_, faceInfoManaged->keypoints);
    }

    bool addPoint(float x, float y)
    {
        auto point = env_->NewObject(facePointManaged_->cls, facePointManaged_->constructor);
        env_->SetFloatField(point, facePointManaged_->x, x);
        env_->SetFloatField(point, facePointManaged_->y, y);

        return env_->CallBooleanMethod(facePoints_, arrayListManaged_->add, point);
    }

    void setRect(float xmin, float ymin, float xmax, float ymax)
    {
        env_->SetFloatField(faceRect_, faceRectManaged_->left, xmin);
        env_->SetFloatField(faceRect_, faceRectManaged_->top, ymin);
        env_->SetFloatField(faceRect_, faceRectManaged_->right, xmax);
        env_->SetFloatField(faceRect_, faceRectManaged_->bottom, ymax);
    }

    void setfaceId(int faceId) { env_->SetIntField(faceInfo_, faceInfoManaged_->faceId, faceId); }

    jobject self() { return faceInfo_; }

    gawrs_face::RelativeBoundingBox faceRect() const
    {
        float xmin = env_->GetFloatField(faceRect_, faceRectManaged_->left);
        float ymin = env_->GetFloatField(faceRect_, faceRectManaged_->top);
        float xmax = env_->GetFloatField(faceRect_, faceRectManaged_->right);
        float ymax = env_->GetFloatField(faceRect_, faceRectManaged_->bottom);

        return gawrs_face::RelativeBoundingBox{
            .xmin = xmin,
            .ymin = ymin,
            .width = xmax - xmin,
            .height = ymax - ymin,
        };
    }

    gawrs_face::RelativeKeypoints keypoints() const
    {
        jint len = env_->CallIntMethod(facePoints_, arrayListManaged_->size);
        gawrs_face::RelativeKeypoints keypoints;
        keypoints.reserve(len);
        for (int i = 0; i < len; i++)
        {
            jobject point = env_->CallObjectMethod(facePoints_, arrayListManaged_->get, i);
            float x = env_->GetFloatField(point, facePointManaged_->x);
            float y = env_->GetFloatField(point, facePointManaged_->y);
            keypoints.emplace_back(x, y);
        }

        return keypoints;
    };

    int faceId() const { return env_->GetIntField(faceInfo_, faceInfoManaged_->faceId); }

public:
    JNIEnv* env_;

    std::shared_ptr<FaceInfoManaged> faceInfoManaged_;
    std::shared_ptr<FaceRectManaged> faceRectManaged_;
    std::shared_ptr<FacePointManaged> facePointManaged_;
    std::shared_ptr<ArrayListManaged> arrayListManaged_;

    jobject faceInfo_;
    jobject faceRect_;
    jobject facePoints_;
};

inline auto createFaceInfoManaged(JNIEnv* env)
{
    auto faceInfoManaged = std::make_shared<FaceInfoManaged>(env);
    auto faceRectManaged = std::make_shared<FaceRectManaged>(env);
    auto facePointManaged = std::make_shared<FacePointManaged>(env);
    auto arrayListManaged = std::make_shared<ArrayListManaged>(env);

    return std::make_tuple(faceInfoManaged, faceRectManaged, facePointManaged, arrayListManaged);
}
