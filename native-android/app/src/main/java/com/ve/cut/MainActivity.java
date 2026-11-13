package com.ve.cut;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Bitmap;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.EditText;
import android.widget.HorizontalScrollView;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.text.InputType;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.Locale;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public final class MainActivity extends Activity {
    private static final int REQUEST_MEDIA = 41;
    private static final int BG = Color.rgb(15, 16, 21);
    private static final int PANEL = Color.rgb(25, 27, 35);
    private static final int PANEL_ALT = Color.rgb(34, 36, 46);
    private static final int TEXT = Color.rgb(244, 245, 250);
    private static final int MUTED = Color.rgb(151, 155, 170);
    private static final int ACCENT = Color.rgb(139, 92, 246);

    private TextView mediaHint;
    private ImageView previewImage;
    private View emptyPreview;
    private TextView selectedClip;
    private LinearLayout videoTrackContent;
    private LinearLayout audioTrackContent;
    private final ExecutorService mediaExecutor = Executors.newSingleThreadExecutor();
    private int selectedClipId = -1;
    private long selectedClipDurationMs = 0;
    private TextView playButton;
    private boolean playing;
    private NativeMediaEngine mediaEngine;
    private PluginHost pluginHost;
    private String engineStatus = "Native engine not initialized";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setStatusBarColor(BG);
        getWindow().setNavigationBarColor(BG);
        getWindow().getDecorView().setSystemUiVisibility(0);

        mediaEngine = new NativeMediaEngine();
        engineStatus = mediaEngine.initialize();
        pluginHost = new PluginHost(this);
        setContentView(buildEditor());
        restoreProjectSources();
    }

    private View buildEditor() {
        LinearLayout page = new LinearLayout(this);
        page.setOrientation(LinearLayout.VERTICAL);
        page.setBackgroundColor(BG);

        page.addView(buildTopBar(), new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(62)));

        ScrollView scroll = new ScrollView(this);
        scroll.setFillViewport(true);
        scroll.setClipToPadding(false);
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setPadding(dp(16), dp(10), dp(16), dp(18));

        content.addView(buildPreview(), new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(226)));
        content.addView(buildPreviewControls(), marginParams(48, 8, 0, 14));
        content.addView(buildTimelineHeader(), marginParams(
                ViewGroup.LayoutParams.MATCH_PARENT, 42, 0, 8));
        content.addView(buildTimeline(), new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(184)));
        content.addView(buildToolStrip(), marginParams(
                ViewGroup.LayoutParams.MATCH_PARENT, 82, 0, 12));
        content.addView(buildStatusPanel(), new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        scroll.addView(content);
        page.addView(scroll, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f));
        return page;
    }

    private View buildTopBar() {
        LinearLayout bar = new LinearLayout(this);
        bar.setGravity(Gravity.CENTER_VERTICAL);
        bar.setPadding(dp(16), 0, dp(14), 0);
        bar.setBackgroundColor(Color.rgb(19, 20, 27));

        TextView mark = label("V", 19, Color.WHITE, true);
        mark.setGravity(Gravity.CENTER);
        mark.setBackground(round(ACCENT, 12));
        bar.addView(mark, new LinearLayout.LayoutParams(dp(36), dp(36)));

        LinearLayout names = new LinearLayout(this);
        names.setOrientation(LinearLayout.VERTICAL);
        names.setPadding(dp(10), 0, 0, 0);
        names.addView(label("VeCut", 17, TEXT, true));
        names.addView(label("UNTITLED PROJECT", 9, MUTED, true));
        bar.addView(names, new LinearLayout.LayoutParams(0,
                ViewGroup.LayoutParams.WRAP_CONTENT, 1f));

        TextView export = action("Export", ACCENT, Color.WHITE);
        export.setOnClickListener(v -> toast("Export is not connected yet."));
        bar.addView(export, new LinearLayout.LayoutParams(dp(88), dp(38)));
        return bar;
    }

    private View buildPreview() {
        FrameLayout frame = new FrameLayout(this);
        frame.setBackground(round(Color.BLACK, 16));
        frame.setClipToOutline(true);

        GLSurfaceView glView = new GLSurfaceView(this);
        int glVersion = supportsEs3() ? 3 : 2;
        glView.setEGLContextClientVersion(glVersion);
        glView.setRenderer(new PreviewRenderer());
        glView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
        frame.addView(glView, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        LinearLayout topLabel = new LinearLayout(this);
        topLabel.setGravity(Gravity.CENTER_VERTICAL);
        topLabel.setPadding(dp(10), dp(6), dp(10), dp(6));
        topLabel.setBackground(round(0xAA171821, 20));
        TextView live = label("●  PREVIEW", 10, 0xFFE3D8FF, true);
        topLabel.addView(live);
        FrameLayout.LayoutParams badgeParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, dp(28), Gravity.TOP | Gravity.LEFT);
        badgeParams.setMargins(dp(10), dp(10), 0, 0);
        frame.addView(topLabel, badgeParams);

        LinearLayout empty = new LinearLayout(this);
        empty.setOrientation(LinearLayout.VERTICAL);
        empty.setGravity(Gravity.CENTER);
        TextView icon = label("▶", 30, 0xFF777B8D, true);
        icon.setGravity(Gravity.CENTER);
        empty.addView(icon);
        mediaHint = label("Your preview will appear here", 13, 0xFFB6B8C4, true);
        mediaHint.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams hintParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        hintParams.topMargin = dp(8);
        empty.addView(mediaHint, hintParams);
        emptyPreview = empty;
        frame.addView(empty, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
        previewImage = new ImageView(this);
        previewImage.setScaleType(ImageView.ScaleType.FIT_CENTER);
        previewImage.setVisibility(View.GONE);
        frame.addView(previewImage, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        TextView resolution = label("OPENGL ES " + glVersion + ".0", 9, 0xFF888B9B, true);
        FrameLayout.LayoutParams resolutionParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.BOTTOM | Gravity.RIGHT);
        resolutionParams.setMargins(0, 0, dp(12), dp(10));
        frame.addView(resolution, resolutionParams);
        return frame;
    }

    private View buildPreviewControls() {
        LinearLayout row = new LinearLayout(this);
        row.setGravity(Gravity.CENTER_VERTICAL);
        TextView time = label("00:00:00", 12, MUTED, true);
        row.addView(time, new LinearLayout.LayoutParams(0, dp(36), 1f));

        playButton = action("▶", PANEL_ALT, TEXT);
        playButton.setGravity(Gravity.CENTER);
        playButton.setOnClickListener(v -> {
            playing = !playing;
            playButton.setText(playing ? "Ⅱ" : "▶");
            toast(playing ? "Preview playback is not connected yet." : "Preview paused.");
        });
        row.addView(playButton, new LinearLayout.LayoutParams(dp(42), dp(36)));

        TextView duration = label("00:00:00", 12, MUTED, true);
        duration.setGravity(Gravity.RIGHT | Gravity.CENTER_VERTICAL);
        row.addView(duration, new LinearLayout.LayoutParams(0, dp(36), 1f));
        return row;
    }

    private View buildTimelineHeader() {
        LinearLayout row = new LinearLayout(this);
        row.setGravity(Gravity.CENTER_VERTICAL);
        TextView heading = label("TIMELINE", 11, MUTED, true);
        row.addView(heading, new LinearLayout.LayoutParams(0,
                ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        TextView zoomOut = smallAction("−");
        TextView zoom = smallAction("100%");
        TextView zoomIn = smallAction("+");
        row.addView(zoomOut);
        row.addView(zoom);
        row.addView(zoomIn);
        return row;
    }

    private View buildTimeline() {
        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(10), dp(8), dp(10), dp(8));
        panel.setBackground(round(PANEL, 14));

        HorizontalScrollView rulerScroll = new HorizontalScrollView(this);
        rulerScroll.setHorizontalScrollBarEnabled(false);
        LinearLayout ruler = new LinearLayout(this);
        ruler.setGravity(Gravity.CENTER_VERTICAL);
        for (int i = 0; i < 7; i++) {
            TextView tick = label(String.format(Locale.US, "%02d:00", i * 5), 9, MUTED, false);
            ruler.addView(tick, new LinearLayout.LayoutParams(dp(64), dp(20)));
        }
        rulerScroll.addView(ruler);
        panel.addView(rulerScroll);

        videoTrackContent = new LinearLayout(this);
        videoTrackContent.setGravity(Gravity.CENTER_VERTICAL);
        panel.addView(track("V1", "＋  Import video or image", 0xFF3C315B, videoTrackContent),
                marginParams(ViewGroup.LayoutParams.MATCH_PARENT, 48, 0, 7));
        audioTrackContent = new LinearLayout(this);
        audioTrackContent.setGravity(Gravity.CENTER_VERTICAL);
        panel.addView(track("A1", "＋  Import audio", 0xFF29464A, audioTrackContent),
                new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(48)));
        selectedClip = label("No clips yet · Import media to begin", 10, MUTED, false);
        LinearLayout.LayoutParams note = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(22));
        note.topMargin = dp(5);
        panel.addView(selectedClip, note);
        return panel;
    }

    private View track(String trackName, String placeholder, int tint, LinearLayout clips) {
        LinearLayout row = new LinearLayout(this);
        row.setGravity(Gravity.CENTER_VERTICAL);
        TextView name = label(trackName, 10, MUTED, true);
        row.addView(name, new LinearLayout.LayoutParams(dp(28), dp(42)));
        HorizontalScrollView scroll = new HorizontalScrollView(this);
        scroll.setHorizontalScrollBarEnabled(false);
        if (clips.getChildCount() == 0) {
            TextView empty = label(placeholder, 11, 0xFFDCD8E8, true);
            empty.setGravity(Gravity.CENTER_VERTICAL);
            empty.setPadding(dp(10), 0, dp(10), 0);
            empty.setBackground(round(tint, 8));
            empty.setOnClickListener(v -> openMediaPicker());
            clips.addView(empty, new LinearLayout.LayoutParams(
                    ViewGroup.LayoutParams.WRAP_CONTENT, dp(38)));
        }
        scroll.addView(clips);
        row.addView(scroll, new LinearLayout.LayoutParams(0, dp(42), 1f));
        return row;
    }

    private void addClipToTrack(int clipId, String name, String mimeType, MediaProbe.Info info, String sourceUri) {
        boolean audio = mimeType != null && mimeType.startsWith("audio/");
        LinearLayout target = audio ? audioTrackContent : videoTrackContent;
        if (target == null) return;
        if (target.getChildCount() == 1 && target.getChildAt(0).getTag() == null) {
            target.removeAllViews();
        }
        String duration = info.durationMs > 0 ? formatTime(info.durationMs) : "Unknown length";
        String dimensions = info.width > 0 && info.height > 0
                ? info.width + "×" + info.height : (mimeType == null ? "Media" : mimeType);
        String title = name == null || name.isEmpty() ? "Media clip" : name;
        TextView chip = label(title + "\n" + duration, 10, TEXT, true);
        chip.setGravity(Gravity.CENTER);
        chip.setPadding(dp(10), dp(3), dp(10), dp(3));
        chip.setBackground(round(audio ? 0xFF31545A : 0xFF514071, 8));
        chip.setTag(clipId);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(dp(124), dp(38));
        params.setMargins(0, 0, dp(6), 0);
        target.addView(chip, params);
        chip.setOnClickListener(v -> {
            selectedClipId = clipId;
            selectedClipDurationMs = info.durationMs;
            selectedClip.setText("Selected #" + clipId + " · " + title
                    + " · " + dimensions + " · " + duration);
        });
        selectedClipId = clipId;
        selectedClipDurationMs = info.durationMs;
        chip.setOnLongClickListener(v -> {
            new AlertDialog.Builder(this).setTitle("Remove clip?")
                    .setMessage("Remove " + title + " from this timeline?")
                    .setNegativeButton("Cancel", null)
                    .setPositiveButton("Remove", (dialog, which) -> {
                        if (mediaEngine.removeClip(clipId)) {
                            target.removeView(chip);
                            ProjectStore.remove(getApplicationContext(), sourceUri);
                            if (selectedClipId == clipId) {
                                selectedClipId = -1;
                                selectedClipDurationMs = 0;
                            }
                            selectedClip.setText(mediaEngine.getTimelineSummary());
                            toast("Clip removed from timeline.");
                        } else toast("Could not remove this clip.");
                    }).show();
            return true;
        });
    }

    private void showTrimDialog() {
        if (selectedClipId < 0) { toast("Select a timeline clip first."); return; }
        LinearLayout fields = new LinearLayout(this);
        fields.setOrientation(LinearLayout.VERTICAL);
        fields.setPadding(dp(20), dp(12), dp(20), 0);
        EditText start = new EditText(this);
        start.setSingleLine(true);
        start.setInputType(InputType.TYPE_CLASS_NUMBER);
        start.setHint("Start (milliseconds)");
        start.setText("0");
        EditText end = new EditText(this);
        end.setSingleLine(true);
        end.setInputType(InputType.TYPE_CLASS_NUMBER);
        end.setHint("End (milliseconds)");
        if (selectedClipDurationMs > 0) end.setText(String.valueOf(selectedClipDurationMs));
        fields.addView(start);
        fields.addView(end);
        new AlertDialog.Builder(this).setTitle("Trim selected clip")
                .setMessage("Enter the source range in milliseconds.")
                .setView(fields).setNegativeButton("Cancel", null)
                .setPositiveButton("Apply", (dialog, which) -> {
                    try {
                        long inMs = Long.parseLong(start.getText().toString().trim());
                        long outMs = Long.parseLong(end.getText().toString().trim());
                        if (selectedClipDurationMs > 0 && outMs > selectedClipDurationMs) {
                            toast("End time exceeds the media duration."); return;
                        }
                        if (!mediaEngine.trimClip(selectedClipId, inMs, outMs)) {
                            toast("Invalid trim range."); return;
                        }
                        selectedClip.setText("Clip #" + selectedClipId + " · Trim "
                                + formatTime(inMs) + " – " + formatTime(outMs));
                        toast("Trim range saved to the native timeline.");
                    } catch (NumberFormatException error) {
                        toast("Enter valid millisecond values.");
                    }
                }).show();
    }
    private String formatTime(long milliseconds) {
        long seconds = Math.max(0, milliseconds / 1000);
        return String.format(Locale.US, "%02d:%02d", seconds / 60, seconds % 60);
    }

    private View buildToolStrip() {
        HorizontalScrollView scroll = new HorizontalScrollView(this);
        scroll.setHorizontalScrollBarEnabled(false);
        LinearLayout tools = new LinearLayout(this);
        tools.setGravity(Gravity.CENTER_VERTICAL);
        tools.addView(tool("＋", "Media", true));
        tools.addView(tool("✂", "Trim", false));
        tools.addView(tool("T", "Text", false));
        tools.addView(tool("◐", "Effects", false));
        tools.addView(tool("♫", "Audio", false));
        tools.addView(tool("▧", "Overlay", false));
        scroll.addView(tools);
        return scroll;
    }

    private View tool(String glyph, String name, boolean primary) {
        LinearLayout item = new LinearLayout(this);
        item.setOrientation(LinearLayout.VERTICAL);
        item.setGravity(Gravity.CENTER);
        item.setPadding(dp(13), dp(9), dp(13), dp(7));
        item.setBackground(round(primary ? 0xFF30264A : PANEL, 12));
        TextView symbol = label(glyph, 19, primary ? 0xFFE5D9FF : TEXT, true);
        symbol.setGravity(Gravity.CENTER);
        item.addView(symbol);
        TextView caption = label(name, 10, primary ? 0xFFE5D9FF : MUTED, true);
        item.addView(caption);
        item.setOnClickListener(v -> {
            if ("Media".equals(name)) {
                openMediaPicker();
            } else if ("Trim".equals(name)) {
                showTrimDialog();
            } else {
                toast(name + " tools are not implemented yet.");
            }
        });
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(dp(70), dp(72));
        params.setMargins(0, 0, dp(8), 0);
        item.setLayoutParams(params);
        return item;
    }

    private View buildStatusPanel() {
        LinearLayout panel = new LinearLayout(this);
        panel.setOrientation(LinearLayout.VERTICAL);
        panel.setPadding(dp(12), dp(10), dp(12), dp(10));
        panel.setBackground(round(PANEL, 12));
        panel.addView(label("ENGINE STATUS", 9, MUTED, true));
        panel.addView(label(engineStatus + " · " + mediaEngine.getClipCount() + " timeline clips",
                11, 0xFFB9BBC7, false));
        TextView plugin = label("Developer plugins: JavaScript runtime available", 10, 0xFF9D91C9, false);
        LinearLayout.LayoutParams pluginParams = new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        pluginParams.topMargin = dp(5);
        panel.addView(plugin, pluginParams);
        return panel;
    }

    private void openMediaPicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_MIME_TYPES, new String[] {
                "video/*", "image/*", "audio/*"
        });
        startActivityForResult(intent, REQUEST_MEDIA);
    }

    private void restoreProjectSources() {
        java.util.List<ProjectStore.Source> sources =
                ProjectStore.load(getApplicationContext());
        if (sources.isEmpty()) return;
        mediaHint.setText("Restoring project media…");
        mediaExecutor.execute(() -> {
            for (ProjectStore.Source source : sources) {
                Uri uri = Uri.parse(source.uri);
                MediaProbe.Info info = MediaProbe.inspect(getApplicationContext(), uri);
                Bitmap frame = MediaProbe.extractPreview(getApplicationContext(), uri, source.mimeType);
                int clipId = mediaEngine.addClip(source.uri, source.mimeType);
                if (clipId < 0) continue;
                runOnUiThread(() -> {
                    if (isFinishing() || isDestroyed()) return;
                    addClipToTrack(clipId, source.displayName, source.mimeType, info, source.uri);
                    showPreviewFrame(frame);
                    mediaHint.setText("Project media restored");
                });
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_MEDIA || resultCode != RESULT_OK || data == null) return;
        Uri uri = data.getData();
        if (uri == null) return;
        try {
            getContentResolver().takePersistableUriPermission(uri,
                    Intent.FLAG_GRANT_READ_URI_PERMISSION);
        } catch (SecurityException ignored) {
            // Some document providers do not offer persistable grants.
        }
        final String mimeType = getContentResolver().getType(uri);
        final String name = uri.getLastPathSegment();
        mediaHint.setText("Reading media information…");
        mediaExecutor.execute(() -> {
            MediaProbe.Info info = MediaProbe.inspect(getApplicationContext(), uri);
            Bitmap frame = MediaProbe.extractPreview(getApplicationContext(), uri, mimeType);
            int clipId = mediaEngine.addClip(uri.toString(), mimeType);
            runOnUiThread(() -> {
                if (isFinishing() || isDestroyed()) return;
                if (clipId < 0) {
                    mediaHint.setText("Could not add this media source");
                    toast("The native timeline could not add this file.");
                    return;
                }
                String displayName = name == null ? "Media file" : name;
                mediaHint.setText("Imported: " + displayName);
                ProjectStore.add(getApplicationContext(), uri.toString(), mimeType, displayName);
                addClipToTrack(clipId, displayName, mimeType, info, uri.toString());
                showPreviewFrame(frame);
                toast("Imported media · Clip #" + clipId);
            });
        });
    }

    @Override
    protected void onDestroy() {
        mediaExecutor.shutdown();
        super.onDestroy();
    }

    private void showPreviewFrame(Bitmap bitmap) {
        if (bitmap == null || previewImage == null) return;
        previewImage.setImageBitmap(bitmap);
        previewImage.setVisibility(View.VISIBLE);
        if (emptyPreview != null) emptyPreview.setVisibility(View.GONE);
    }

    private boolean supportsEs3() {
        android.app.ActivityManager manager =
                (android.app.ActivityManager) getSystemService(ACTIVITY_SERVICE);
        android.content.pm.ConfigurationInfo info = manager.getDeviceConfigurationInfo();
        return info != null && info.reqGlEsVersion >= 0x30000;
    }

    private TextView smallAction(String text) {
        TextView view = action(text, PANEL_ALT, TEXT);
        view.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(dp(42), dp(30));
        params.setMargins(dp(4), 0, 0, 0);
        view.setLayoutParams(params);
        return view;
    }

    private TextView action(String text, int color, int foreground) {
        TextView view = label(text, 12, foreground, true);
        view.setGravity(Gravity.CENTER);
        view.setBackground(round(color, 10));
        return view;
    }

    private TextView label(String text, float size, int color, boolean bold) {
        TextView view = new TextView(this);
        view.setText(text);
        view.setTextSize(size);
        view.setTextColor(color);
        if (bold) view.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        view.setIncludeFontPadding(false);
        return view;
    }

    private GradientDrawable round(int color, int radius) {
        GradientDrawable drawable = new GradientDrawable();
        drawable.setColor(color);
        drawable.setCornerRadius(dp(radius));
        return drawable;
    }

    private LinearLayout.LayoutParams marginParams(int width, int height, int top, int bottom) {
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(width,
                height == ViewGroup.LayoutParams.WRAP_CONTENT ? height : dp(height));
        params.topMargin = dp(top);
        params.bottomMargin = dp(bottom);
        return params;
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }

    private void toast(String message) {
        Toast.makeText(this, message, Toast.LENGTH_SHORT).show();
    }

    private static final class PreviewRenderer implements GLSurfaceView.Renderer {
        @Override public void onSurfaceCreated(javax.microedition.khronos.opengles.GL10 gl,
                                                javax.microedition.khronos.egl.EGLConfig config) {
            GLES20.glClearColor(0.035f, 0.038f, 0.052f, 1.0f);
        }

        @Override public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl,
                                               int width, int height) {
            GLES20.glViewport(0, 0, width, height);
        }

        @Override public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
        }
    }
}
