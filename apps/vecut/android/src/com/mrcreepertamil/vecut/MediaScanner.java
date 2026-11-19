package com.mrcreepertamil.vecut;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.MediaStore;

import java.util.ArrayList;
import java.util.LinkedHashSet;

public final class MediaScanner {
    private MediaScanner() {}

    public static String[] scan(Context context) {
        LinkedHashSet<String> paths = new LinkedHashSet<>();
        ContentResolver resolver = context.getContentResolver();

        scanCollection(resolver, MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                MediaStore.Images.Media.DATA, paths);
        scanCollection(resolver, MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                MediaStore.Video.Media.DATA, paths);
        scanCollection(resolver, MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
                MediaStore.Audio.Media.DATA, paths);

        return paths.toArray(new String[0]);
    }

    private static void scanCollection(ContentResolver resolver, Uri collection,
                                       String dataColumn, LinkedHashSet<String> paths) {
        String[] projection = new String[] {
                dataColumn,
                MediaStore.MediaColumns.DISPLAY_NAME,
                MediaStore.MediaColumns.DATE_ADDED
        };
        try (Cursor cursor = resolver.query(collection, projection, null, null,
                MediaStore.MediaColumns.DATE_ADDED + " DESC")) {
            if (cursor == null) return;
            int dataIndex = cursor.getColumnIndex(dataColumn);
            if (dataIndex < 0) return;
            while (cursor.moveToNext()) {
                String path = cursor.getString(dataIndex);
                if (path != null && !path.isEmpty()) paths.add(path);
            }
        } catch (SecurityException | IllegalArgumentException ignored) {
            // A missing permission or unsupported provider column should not crash the app.
        }
    }
}
