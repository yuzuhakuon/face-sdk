package com.gawrs.face;

public enum RotationModel {
    ROTATION_0(0),
    ROTATION_90(1),
    ROTATION_180(2),
    ROTATION_270(3);

    private final int value;

    RotationModel(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
