package com.ve.cut;

/** Java-facing JNI API for VeCut's native timeline and media engine. */
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

    /** Sets the full source duration when it is known from Android metadata. */
    public boolean setClipDuration(int clipId, long durationMs) {
        return nativeSetClipDuration(clipId, durationMs);
    }

    /** Sets a source-time range in milliseconds; end must be greater than start. */
    public boolean trimClip(int clipId, long sourceInMs, long sourceOutMs) {
        return nativeTrimClip(clipId, sourceInMs, sourceOutMs);
    }

    /** Stores an incoming transition on a clip. Type: 0 none, 1 fade, 2 dissolve, 3/4 wipe, 5/6 slide. */
    public boolean setClipTransition(int clipId, int type, long durationMs) {
        return nativeSetClipTransition(clipId, type, durationMs);
    }

    /** Returns [sourceInMs, sourceOutMs, transitionType, transitionDurationMs]. */
    public long[] getClipSettings(int clipId) {
        return nativeGetClipSettings(clipId);
    }

    /** Moves a clip to a zero-based position in the timeline. */
    public boolean moveClip(int clipId, int targetIndex) {
        return nativeMoveClip(clipId, targetIndex);
    }

    /** Splits a clip at source time and returns the new right-hand clip ID. */
    public int splitClip(int clipId, long splitAtMs) {
        return nativeSplitClip(clipId, splitAtMs);
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
    private native boolean nativeSetClipDuration(int clipId, long durationMs);
    private native boolean nativeSetClipTransition(int clipId, int type, long durationMs);
    private native long[] nativeGetClipSettings(int clipId);
    private native boolean nativeTrimClip(int clipId, long sourceInMs, long sourceOutMs);
    private native boolean nativeMoveClip(int clipId, int targetIndex);
    private native int nativeSplitClip(int clipId, long splitAtMs);
    private native int nativeGetClipCount();
    private native String nativeGetTimelineSummary();
    private native void nativeClearTimeline();
}
