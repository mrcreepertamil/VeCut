package com.ve.cut;

/** Java-facing JNI API for VeCut's native timeline and future media engine. */
public final class NativeMediaEngine {
    static {
        System.loadLibrary("vecut_engine");
    }

    public String initialize() {
        return nativeInitialize();
    }

    /** Adds a selected source to the native timeline and returns its clip ID. */
    public int addClip(String uri, String mimeType) {
        if (uri == null || uri.isEmpty()) return -1;
        return nativeAddClip(uri, mimeType == null ? "" : mimeType);
    }

    public boolean removeClip(int clipId) {
        return nativeRemoveClip(clipId);
    }

    /** Sets a source-time range in milliseconds; end must be greater than start. */
    public boolean trimClip(int clipId, long sourceInMs, long sourceOutMs) {
        return nativeTrimClip(clipId, sourceInMs, sourceOutMs);
    }

    public int getClipCount() {
        return nativeGetClipCount();
    }

    public String getTimelineSummary() {
        return nativeGetTimelineSummary();
    }

    public void clearTimeline() {
        nativeClearTimeline();
    }

    private native String nativeInitialize();
    private native int nativeAddClip(String uri, String mimeType);
    private native boolean nativeRemoveClip(int clipId);
    private native boolean nativeTrimClip(int clipId, long sourceInMs, long sourceOutMs);
    private native int nativeGetClipCount();
    private native String nativeGetTimelineSummary();
    private native void nativeClearTimeline();
}
