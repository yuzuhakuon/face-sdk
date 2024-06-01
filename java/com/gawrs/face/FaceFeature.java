package com.gawrs.face;

public class FaceFeature {
    /**
     * feature data size , 512*4 + 8*4
     */
    public static final int FEATURE_SIZE = 2080;

    /**
     * feature data
     */
    public byte[] featureData;

    public FaceFeature() {
        this.featureData = new byte[FEATURE_SIZE];
    }

    public FaceFeature(FaceFeature obj) {
        if (obj == null) {
            featureData = new byte[FEATURE_SIZE];
        } else {
            featureData = obj.featureData.clone();
        }
    }

    /**
     * deep copy
     */
    @Override
    public FaceFeature clone() {
        return new FaceFeature(this);
    }
}
