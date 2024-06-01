/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_gawrs_face_FaceKit */

#ifndef _Included_com_gawrs_face_FaceKit
#define _Included_com_gawrs_face_FaceKit
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_gawrs_face_FaceKit
 * Method:    initFaceEngine
 * Signature: (IIIFFI)I
 */
JNIEXPORT jint JNICALL Java_com_gawrs_face_FaceKit_initFaceEngine
  (JNIEnv *, jobject, jint, jint, jint, jfloat, jfloat, jint);

/*
 * Class:     com_gawrs_face_FaceKit
 * Method:    detectFaces
 * Signature: (J[BIIILjava/util/ArrayList;)I
 */
JNIEXPORT jint JNICALL Java_com_gawrs_face_FaceKit_detectFaces
  (JNIEnv *, jobject, jlong, jbyteArray, jint, jint, jint, jobject);

/*
 * Class:     com_gawrs_face_FaceKit
 * Method:    extractFaceFeature
 * Signature: (J[BIIILcom/gawrs/face/FaceInfo;Lcom/gawrs/face/FaceFeature;)I
 */
JNIEXPORT jint JNICALL Java_com_gawrs_face_FaceKit_extractFaceFeature
  (JNIEnv *, jobject, jlong, jbyteArray, jint, jint, jint, jobject, jobject);

/*
 * Class:     com_gawrs_face_FaceKit
 * Method:    compareFaceFeature
 * Signature: (JLcom/gawrs/face/FaceFeature;Lcom/gawrs/face/FaceFeature;Lcom/gawrs/face/FaceSimilar;)I
 */
JNIEXPORT jint JNICALL Java_com_gawrs_face_FaceKit_compareFaceFeature
  (JNIEnv *, jobject, jlong, jobject, jobject, jobject);

/*
 * Class:     com_gawrs_face_FaceKit
 * Method:    unInitFaceEngine
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_com_gawrs_face_FaceKit_unInitFaceEngine
  (JNIEnv *, jobject, jlong);

/*
 * Class:     com_gawrs_face_FaceKit
 * Method:    getVersionInfo
 * Signature: (Lcom/gawrs/face/VersionInfo;)V
 */
JNIEXPORT void JNICALL Java_com_gawrs_face_FaceKit_getVersionInfo
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif
