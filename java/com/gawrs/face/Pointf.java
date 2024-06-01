package com.gawrs.face;

public class Pointf {
    public float x;

    public float y;

    public Pointf() {
    }

    public Pointf(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public Pointf(Pointf p) {
        if (p == null) {
            this.x = this.y = 0;
        } else {
            this.x = p.x;
            this.y = p.y;
        }
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder(32);
        sb.append("Pointf(");
        sb.append(this.x);
        sb.append(", ");
        sb.append(this.y);
        sb.append(")");
        return sb.toString();
    }
}
