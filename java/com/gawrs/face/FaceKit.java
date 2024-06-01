package com.gawrs.face;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;

class FaceKit {
    private static List<String> libCache = new LinkedList<String>();
    private static final String FACE_SDK_LIBRARY = "gawrs_face_jsdk";

    private long handle = 0L;

    private static synchronized void libraryLoad(String libPath) {
        if (libPath == null || libPath.length() == 0) {
            if (!libCache.contains(FACE_SDK_LIBRARY)) {
                System.loadLibrary(FACE_SDK_LIBRARY);
                libCache.add(FACE_SDK_LIBRARY);
            }
        } else {
            if (libPath.endsWith("/") || libPath.endsWith("\\")) {
                libPath = libPath.substring(0, libPath.length() - 1);
            }
            if (!libCache.contains(libPath)) {
                System.out.println("load library: " + libPath);
                System.load(libPath);
                libCache.add(libPath);
            }
        }
    }

    public FaceKit() {
        libraryLoad(null);
    }

    public FaceKit(String libPath) {
        libraryLoad(libPath);
    }

    private native int initFaceEngine(
            int detectFaceScaleVal,
            int detectFaceMaxNum,
            int combinedMask,
            float probThreshold,
            float nmsThreshold,
            int rotation);

    private native int detectFaces(
            long handle,
            byte[] data,
            int width,
            int height,
            int format,
            ArrayList<FaceInfo> faceInfoList);

    private native int extractFaceFeature(
            long handle,
            byte[] data,
            int width,
            int height,
            int format,
            FaceInfo faceInfo,
            FaceFeature feature);

    private native int compareFaceFeature(
            long handle,
            FaceFeature feature1,
            FaceFeature feature2,
            FaceSimilar faceSimilar);

    private native int unInitFaceEngine(long handle);

    private native void getVersionInfo(VersionInfo versionInfo);

    public int init(EngineConfiguration configuration) {
        if (configuration == null) {
            return ErrorInfo.GFE_INVALID_PARAM;
        }

        int detectFaceScaleVal = configuration.getDetectFaceScaleVal();
        int detectFaceMaxNum = configuration.getDetectFaceMaxNum();
        float probThreshold = configuration.getProbThreshold();
        float nmsThreshold = configuration.getNmsThreshold();
        int rotation = configuration.getRotation().getValue();
        int combinedMask = 0;
        if (configuration.isSupportFaceDetect()) {
            combinedMask |= RuntimeModel.FACE_DETECT;
        }
        if (configuration.isSupportFaceRecognition()) {
            combinedMask |= RuntimeModel.FACE_RECOGNITION;
        }

        int errCode = initFaceEngine(detectFaceScaleVal, detectFaceMaxNum, combinedMask, probThreshold, nmsThreshold,
                rotation);
        return errCode;
    }

    public int detectFaces(ImageInfo image, ArrayList<FaceInfo> faceInfos) {
        if (handle == 0L) {
            return ErrorInfo.GFE_ENGINE_NOT_INIT;
        }
        if (image == null) {
            return ErrorInfo.GFE_INVALID_IMAGE;
        }

        byte[] data = image.getImageData();
        int width = image.getWidth();
        int height = image.getHeight();
        int format = image.getImageFormat();

        return detectFaces(handle, data, width, height, format, faceInfos);
    }

    public int extractFaceFeature(ImageInfo image, FaceInfo faceInfo, FaceFeature faceFeature) {
        if (handle == 0L) {
            return ErrorInfo.GFE_ENGINE_NOT_INIT;
        }

        if (image == null || faceInfo == null || faceFeature == null) {
            return ErrorInfo.GFE_INVALID_PARAM;
        }

        byte[] data = image.getImageData();
        int width = image.getWidth();
        int height = image.getHeight();
        int format = image.getImageFormat();

        if (data == null || width < 100 || height < 100) {
            return ErrorInfo.GFE_INVALID_IMAGE;
        }

        return extractFaceFeature(handle, data, width, height, format, faceInfo, faceFeature);
    }

    public int compareFaceFeature(FaceFeature faceFeature1, FaceFeature faceFeature2, FaceSimilar faceSimilar) {
        if (faceFeature1 == null || faceFeature2 == null || faceSimilar == null) {
            return ErrorInfo.GFE_INVALID_PARAM;
        }

        if (faceFeature1.featureData == null || faceFeature2.featureData == null) {
            return ErrorInfo.GFE_INVALID_PARAM;
        }

        return compareFaceFeature(handle, faceFeature1, faceFeature2, faceSimilar);
    }

    public int unInit() {
        int errorCode = ErrorInfo.GFE_OK;
        if (handle != 0L) {
            errorCode = unInitFaceEngine(handle);
        }
        return errorCode;
    }

    public VersionInfo getVersionInfo() {
        VersionInfo versionInfo = new VersionInfo();
        getVersionInfo(versionInfo);
        return versionInfo;
    }

    public long getHandle() {
        return handle;
    }
}