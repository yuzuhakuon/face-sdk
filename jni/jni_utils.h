#pragma once

#include <jni.h>
#include <vector>

static std::vector<unsigned char> byteArray2Vector(JNIEnv* env, jbyteArray arr)
{
    jbyte* body = env->GetByteArrayElements(arr, 0);
    int len = env->GetArrayLength(arr);
    std::vector<unsigned char> res(len);
    std::memcpy(res.data(), body, len * sizeof(char));
    env->ReleaseByteArrayElements(arr, body, 0);
    return res;
}