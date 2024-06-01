package com.gawrs.face;

public class ImageInfo {
    private byte[] imageData;
    private int width;
    private int height;
    private int imageFormat;

    public byte[] getImageData() {
        if (imageData == null) {
            imageData = new byte[0];
        }
        return imageData;
    }

    public void setImageData(byte[] imageData) {
        this.imageData = imageData;
    }

    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getImageFormat() {
        return imageFormat;
    }

    public void setImageFormat(int imageFormat) {
        this.imageFormat = imageFormat;
    }
}
