package com.gawrs.face;

public class Rectf {
    public float left;

    public float top;

    public float right;

    public float bottom;

    public Rectf() {
    }

    public Rectf(float left, float top, float right, float bottom) {
        this.left = left;
        this.top = top;
        this.right = right;
        this.bottom = bottom;
    }

    public Rectf(Rectf r) {
        if (r == null) {
            this.left = this.top = this.right = this.bottom = 0;
        } else {
            this.left = r.left;
            this.top = r.top;
            this.right = r.right;
            this.bottom = r.bottom;
        }
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder(32);
        sb.append("Rectf(");
        sb.append(this.left);
        sb.append(", ");
        sb.append(this.top);
        sb.append(", ");
        sb.append(this.right);
        sb.append(", ");
        sb.append(this.bottom);
        sb.append(")");
        return sb.toString();
    }
}
