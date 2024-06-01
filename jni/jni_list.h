#pragma once

#include <jni.h>
#include <memory>
#include <vector>

class ArrayListManaged
{
public:
    ArrayListManaged(JNIEnv* env)
    {
        cls = env->FindClass(kSignature);

        constructor = env->GetMethodID(cls, "<init>", "()V");
        size = env->GetMethodID(cls, "size", "()I");
        get = env->GetMethodID(cls, "get", "(I)Ljava/lang/Object;");
        add = env->GetMethodID(cls, "add", "(Ljava/lang/Object;)Z");
    }

public:
    static constexpr char kSignature[] = "java/util/ArrayList";

    // Class
    jclass cls;

    // Method
    jmethodID constructor;
    jmethodID size;
    jmethodID get;
    jmethodID add;
};

class JArrayList
{
public:
    JArrayList(std::shared_ptr<ArrayListManaged> mManaged) : managed_(mManaged) {}

    static std::shared_ptr<JArrayList> fromJava(std::shared_ptr<ArrayListManaged> managed, JNIEnv* env, jobject obj)
    {
        auto res = std::make_shared<JArrayList>(managed);
        jint len = env->CallIntMethod(obj, managed->size);
        res->elements_.reserve(len);

        for (int i = 0; i < len; i++)
        {
            jobject element = env->CallObjectMethod(obj, managed->get);
            res->elements_.push_back(element);
        }

        return res;
    }

    static jobject toJava(std::shared_ptr<ArrayListManaged> managed, std::shared_ptr<JArrayList> cobj, JNIEnv* env)
    {
        auto jobj = env->NewObject(managed->cls, managed->constructor);
        toJava(managed, cobj, env, jobj);
        return jobj;
    }

    static void toJava(std::shared_ptr<ArrayListManaged> managed, std::shared_ptr<JArrayList> cobj, JNIEnv* env,
                       jobject jobj)
    {
        for (auto element : cobj->elements_)
        {
            env->CallBooleanMethod(jobj, managed->add, element);
        }
    }

public:
    std::vector<jobject>& elements() { return elements_; }

    int size() const { return elements_.size(); }

private:
    std::shared_ptr<ArrayListManaged> managed_;

    std::vector<jobject> elements_;
};

class JArrayListRef
{
public:
    JArrayListRef(std::shared_ptr<ArrayListManaged> managed, JNIEnv* env, jobject obj)
        : managed_(managed), env_(env), thiz(obj)
    {
    }

    JArrayListRef(JNIEnv* env, std::shared_ptr<ArrayListManaged> managed, jobject obj)
        : managed_(managed), env_(env), thiz(obj)
    {
    }

    int size() const
    {
        jint len = env_->CallIntMethod(thiz, managed_->size);
        return len;
    }

    jobject get(int index)
    {
        jobject element = env_->CallObjectMethod(thiz, managed_->get, index);
        return element;
    }

    bool add(jobject element) { return env_->CallBooleanMethod(thiz, managed_->add, element); }

private:
    JNIEnv* env_;
    std::shared_ptr<ArrayListManaged> managed_;

    jobject thiz;
};
