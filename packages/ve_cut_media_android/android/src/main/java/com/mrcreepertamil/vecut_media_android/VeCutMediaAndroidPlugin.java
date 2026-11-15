package com.mrcreepertamil.vecut_media_android;

import android.media.MediaMetadataRetriever;

import androidx.annotation.NonNull;

import java.util.HashMap;
import java.util.Map;

import io.flutter.embedding.engine.plugins.FlutterPlugin;
import io.flutter.plugin.common.MethodCall;
import io.flutter.plugin.common.MethodChannel;
import io.flutter.plugin.common.MethodChannel.MethodCallHandler;
import io.flutter.plugin.common.MethodChannel.Result;

/**
 * Provides Android-native media inspection for VeCut.
 */
public class VeCutMediaAndroidPlugin implements FlutterPlugin, MethodCallHandler {
    private MethodChannel channel;

    @Override
    public void onAttachedToEngine(@NonNull FlutterPluginBinding binding) {
        channel = new MethodChannel(
            binding.getBinaryMessenger(),
            "ve_cut_media_android"
        );
        channel.setMethodCallHandler(this);
    }

    @Override
    public void onMethodCall(@NonNull MethodCall call, @NonNull Result result) {
        if (!"getVideoInfo".equals(call.method)) {
            result.notImplemented();
            return;
        }

        String path = call.argument("path");
        if (path == null || path.trim().isEmpty()) {
            result.error("INVALID_PATH", "A video file path is required.", null);
            return;
        }

        MediaMetadataRetriever retriever = new MediaMetadataRetriever();
        try {
            retriever.setDataSource(path);

            String durationValue = retriever.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_DURATION
            );
            String widthValue = retriever.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH
            );
            String heightValue = retriever.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT
            );
            String rotationValue = retriever.extractMetadata(
                MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION
            );

            Map<String, Object> info = new HashMap<>();
            info.put("durationMs", parseLong(durationValue));
            info.put("width", parseInt(widthValue));
            info.put("height", parseInt(heightValue));
            info.put("rotation", parseInt(rotationValue));

            result.success(info);
        } catch (Exception exception) {
            result.error(
                "MEDIA_READ_FAILED",
                "VeCut could not read this video file.",
                exception.getMessage()
            );
        } finally {
            try {
                retriever.release();
            } catch (Exception ignored) {
                // Nothing else to release if metadata reading already failed.
            }
        }
    }

    private long parseLong(String value) {
        try {
            return value == null ? 0L : Long.parseLong(value);
        } catch (NumberFormatException ignored) {
            return 0L;
        }
    }

    private int parseInt(String value) {
        try {
            return value == null ? 0 : Integer.parseInt(value);
        } catch (NumberFormatException ignored) {
            return 0;
        }
    }

    @Override
    public void onDetachedFromEngine(@NonNull FlutterPluginBinding binding) {
        if (channel != null) {
            channel.setMethodCallHandler(null);
            channel = null;
        }
    }
}
