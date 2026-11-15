package com.ve.cut;

import android.content.ContentResolver;
import android.content.Context;

/** Small application-context holder used by the decoder worker. */
final class AppContextHolder {
    private static volatile ContentResolver resolver;
    private AppContextHolder() {}
    static void initialize(Context context) {
        resolver = context.getApplicationContext().getContentResolver();
    }
    static ContentResolver getResolver() { return resolver; }
}
