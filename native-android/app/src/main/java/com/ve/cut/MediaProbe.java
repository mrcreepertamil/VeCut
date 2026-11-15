package com.ve.cut;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.MediaMetadataRetriever;
import android.net.Uri;

import java.io.InputStream;

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
            catch (Exception ignored) { /* Best-effort release, including platform IO failures. */ }
        }
        if (rotation == 90 || rotation == 270) {
            int swap = width;
            width = height;
            height = swap;
        }
        return new Info(duration, width, height, rotation);
    }

    /** Extracts a still preview frame for images and videos; audio has no visual frame. */
    public static Bitmap extractPreview(Context context, Uri uri, String mimeType) {
        Bitmap bitmap = null;
        if (mimeType != null && mimeType.startsWith("image/")) {
            try (InputStream input = context.getContentResolver().openInputStream(uri)) {
                if (input != null) {
                    BitmapFactory.Options bounds = new BitmapFactory.Options();
                    bounds.inJustDecodeBounds = true;
                    BitmapFactory.decodeStream(input, null, bounds);
                    int sample = 1;
                    while (bounds.outWidth / sample > 1280 || bounds.outHeight / sample > 720) {
                        sample *= 2;
                    }
                    try (InputStream decoded = context.getContentResolver().openInputStream(uri)) {
                        if (decoded != null) {
                            BitmapFactory.Options options = new BitmapFactory.Options();
                            options.inSampleSize = sample;
                            bitmap = BitmapFactory.decodeStream(decoded, null, options);
                        }
                    }
                }
            } catch (Exception ignored) {
                return null;
            }
        } else if (mimeType != null && mimeType.startsWith("video/")) {
            MediaMetadataRetriever retriever = new MediaMetadataRetriever();
            try {
                retriever.setDataSource(context, uri);
                bitmap = retriever.getFrameAtTime(0,
                        MediaMetadataRetriever.OPTION_CLOSEST_SYNC);
            } catch (RuntimeException ignored) {
                return null;
            } finally {
                try { retriever.release(); }
                catch (RuntimeException ignored) { /* Best-effort release. */ }
            }
        }
        if (bitmap != null && (bitmap.getWidth() > 1280 || bitmap.getHeight() > 720)) {
            float scale = Math.min(1280f / bitmap.getWidth(), 720f / bitmap.getHeight());
            Bitmap scaled = Bitmap.createScaledBitmap(bitmap,
                    Math.max(1, Math.round(bitmap.getWidth() * scale)),
                    Math.max(1, Math.round(bitmap.getHeight() * scale)), true);
            if (scaled != bitmap) bitmap.recycle();
            bitmap = scaled;
        }
        return bitmap;
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
