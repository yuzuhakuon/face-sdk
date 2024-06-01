package com.gawrs.face;

public class EngineConfiguration {
    /**
     * Detect face max num
     */
    private Integer detectFaceMaxNum = 4;

    /**
     * Detect face scale value
     */
    private Integer detectFaceScaleVal = 10;

    private Float probThreshold = 0.6f;

    private Float nmsThreshold = 0.45f;

    /**
     * Support face detect
     */
    private Boolean supportFaceDetect = false;

    /**
     * Support face recognition
     */
    private Boolean supportFaceRecognition = false;

    /**
     * Image rotation mode
     */
    private RotationModel rotation = RotationModel.ROTATION_0;

    public Integer getDetectFaceMaxNum() {
        return detectFaceMaxNum;
    }

    public void setDetectFaceMaxNum(Integer detectFaceMaxNum) {
        this.detectFaceMaxNum = detectFaceMaxNum;
    }

    public Integer getDetectFaceScaleVal() {
        return detectFaceScaleVal;
    }

    public void setDetectFaceScaleVal(Integer detectFaceScaleVal) {
        this.detectFaceScaleVal = detectFaceScaleVal;
    }

    public Float getProbThreshold() {
        return probThreshold;
    }

    public void setProbThreshold(Float probThreshold) {
        this.probThreshold = probThreshold;
    }

    public Float getNmsThreshold() {
        return nmsThreshold;
    }

    public void setNmsThreshold(Float nmsThreshold) {
        this.nmsThreshold = nmsThreshold;
    }

    public Boolean isSupportFaceDetect() {
        return supportFaceDetect;
    }

    public void setSupportFaceDetect(Boolean supportFaceDetect) {
        this.supportFaceDetect = supportFaceDetect;
    }

    public Boolean isSupportFaceRecognition() {
        return supportFaceRecognition;
    }

    public void setSupportFaceRecognition(Boolean supportFaceRecognition) {
        this.supportFaceRecognition = supportFaceRecognition;
    }

    public RotationModel getRotation() {
        return rotation;
    }

    public void setRotation(RotationModel rotation) {
        this.rotation = rotation;
    }
}
