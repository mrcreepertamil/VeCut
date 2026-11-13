package com.ve.cut;

/** Java-facing JNI bridge to VeCut's native C++ media engine. */
public final class NativeMediaEngine {
    static {
        System.loadLibrary("vecut_engine");
    }

    public String initialize() {
        return nativeInitialize();
    }

    private native String nativeInitialize();
}
