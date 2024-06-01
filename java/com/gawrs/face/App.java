package com.gawrs.face;

import java.io.File;
import java.util.ArrayList;

public class App {
    public static void main(String[] args) {
        String libPath = System.getProperty("user.dir") + "/out/" + System.mapLibraryName("gawrs_face_jsdk");
        FaceKit faceKit = new FaceKit(libPath);

        VersionInfo versionInfo = faceKit.getVersionInfo();
        System.out.println("SDK version: " + versionInfo.version);
        System.out.println("Build date: " + versionInfo.buildDate);
        System.out.println("Copyright: " + versionInfo.copyRight);

        EngineConfiguration config = new EngineConfiguration();
        config.setDetectFaceMaxNum(4);
        config.setDetectFaceScaleVal(12);
        config.setSupportFaceDetect(true);
        config.setSupportFaceRecognition(true);

        int errCode = faceKit.init(config);
        if (errCode != ErrorInfo.GFE_OK) {
            System.out.println("init error: " + errCode);
            return;
        }

        FaceFeature faceFeatureA = makeOneFeature(faceKit, "samples/data/images/a1.png");
        FaceFeature faceFeatureB = makeOneFeature(faceKit, "samples/data/images/a2.png");
        FaceFeature faceFeatureC = makeOneFeature(faceKit, "samples/data/images/b1.png");
        if (faceFeatureA == null && faceFeatureB == null && faceFeatureC == null) {
            System.out.println("extract feature error");
            return;
        }

        FaceSimilar faceSimilarAB = new FaceSimilar();
        FaceSimilar faceSimilarAC = new FaceSimilar();
        FaceSimilar faceSimilarBC = new FaceSimilar();
        int errCodeAB = faceKit.compareFaceFeature(faceFeatureA, faceFeatureB, faceSimilarAB);
        int errCodeAC = faceKit.compareFaceFeature(faceFeatureA, faceFeatureC, faceSimilarAC);
        int errCodeBC = faceKit.compareFaceFeature(faceFeatureB, faceFeatureC, faceSimilarBC);
        if (errCodeAB != ErrorInfo.GFE_OK || errCodeAC != ErrorInfo.GFE_OK || errCodeBC != ErrorInfo.GFE_OK) {
            System.out.println("compare error: " + errCode);
            return;
        }
        System.out.println("face similar AB: " + faceSimilarAB.score);
        System.out.println("face similar AC: " + faceSimilarAC.score);
        System.out.println("face similar BC: " + faceSimilarBC.score);
    }

    public static FaceFeature makeOneFeature(FaceKit faceKit, String imgPath) {
        // read image
        ImageInfo image = ImageFactory.getRGBData(new File(imgPath));

        // face detect
        ArrayList<FaceInfo> faceInfos = new ArrayList<FaceInfo>();
        faceKit.detectFaces(image, faceInfos);
        if (faceInfos.size() > 0) {
            FaceInfo info = faceInfos.get(0);
            System.out.println("face info: " + info.rect);
            FaceFeature faceFeature = new FaceFeature();
            int ret = faceKit.extractFaceFeature(image, info, faceFeature);
            if (ret == ErrorInfo.GFE_OK) {
                return faceFeature;
            }
        }

        return null;
    }
}
