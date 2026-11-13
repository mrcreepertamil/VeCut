package com.ve.cut;

import android.content.Context;
import android.media.MediaMetadataRetriever;
import android.net.Uri;

/** Reads basic media properties using Android's platform metadata extractor. */
public final class MediaProbe {
    private MediaProbe() {}

    public static final class Info {
        public final long durationMs;
        public final int width;
        public final int height;
        public final int rotation;

        Info(long durationMs, int width, int height, int rotation) {
            this.durationMs = durationMs;
            this.width = width;
            this.height = height;
            this.rotation = rotation;
        }
    }

    public static Info inspect(Context context, Uri uri) {
        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        long duration = 0;
        int width = 0;
        int height = 0;
        int rotation = 0;
        try {
            retriever.setDataSource(context, uri);
            duration = parseLong(retriever.extractMetadata(
                    MediaMetadataRetriever.METADATA_KEY_DURATION));
            width = parseInt(retriever.extractMetadata(
                    MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH));
            height = parseInt(retriever.extractMetadata(
                    MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT));
            rotation = parseInt(retriever.extractMetadata(
                    MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION));
        } catch (RuntimeException ignored) {
            // Some document providers expose sources that cannot be inspected.
        } finally {
            try { retriever.release(); }
            catch (RuntimeException ignored) { /* Best-effort release. */ }
        }
        if (rotation == 90 || rotation == 270) {
            int swap = width;
            width = height;
            height = swap;
        }
        return new Info(duration, width, height, rotation);
    }

    private static long parseLong(String value) {
        try { return value == null ? 0 : Long.parseLong(value); }
        catch (NumberFormatException ignored) { return 0; }
    }

    private static int parseInt(String value) {
        try { return value == null ? 0 : Integer.parseInt(value); }
        catch (NumberFormatException ignored) { return 0; }
    }
}
