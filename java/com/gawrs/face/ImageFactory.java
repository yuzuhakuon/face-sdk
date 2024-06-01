package com.gawrs.face;

import java.awt.color.ColorSpace;
import java.awt.image.BufferedImage;
import java.awt.image.ColorConvertOp;
import java.awt.image.DataBufferByte;
import java.io.File;
import java.io.IOException;

import javax.imageio.ImageIO;

public class ImageFactory {
    public static ImageInfo getRGBData(File file) {
        if (file == null)
            return null;
        ImageInfo imageInfo;
        try {
            // load image into buffer
            BufferedImage image = ImageIO.read(file);
            imageInfo = bufferedImage2ImageInfo(image);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
        return imageInfo;
    }

    private static ImageInfo bufferedImage2ImageInfo(BufferedImage image) {
        ImageInfo imageInfo = new ImageInfo();
        int width = image.getWidth();
        int height = image.getHeight();
        // center image
        // width = width & (~3);
        // height = height & (~3);
        imageInfo.setWidth(width);
        imageInfo.setHeight(height);
        // new image buffer
        BufferedImage resultImage = new BufferedImage(width, height, image.getType());
        // rgb pixels data
        int[] rgb = image.getRGB(0, 0, width, height, null, 0, width);
        // draw pixels
        resultImage.setRGB(0, 0, width, height, rgb, 0, width);
        // format
        BufferedImage dstImage = new BufferedImage(width, height, BufferedImage.TYPE_3BYTE_BGR);
        if (resultImage.getType() != BufferedImage.TYPE_3BYTE_BGR) {
            ColorSpace cs = ColorSpace.getInstance(ColorSpace.CS_LINEAR_RGB);
            ColorConvertOp colorConvertOp = new ColorConvertOp(cs, dstImage.createGraphics().getRenderingHints());
            colorConvertOp.filter(resultImage, dstImage);
        } else {
            dstImage = resultImage;
        }
        // rgb data
        imageInfo.setImageFormat(ImageFormat.FS_BGR24);
        imageInfo.setImageData(((DataBufferByte) (dstImage.getRaster().getDataBuffer())).getData());
        return imageInfo;
    }
}
