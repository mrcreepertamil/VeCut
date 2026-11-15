package com.ve.cut;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.ClipData;
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
import android.widget.SeekBar;
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
    private static final int ACCENT = Color.rgb(34, 197, 94);

    private TextView mediaHint;
    private TextView projectTitle;
    private ImageView previewImage;
    private GlVideoPreview videoPreview;
    private AudioPreviewPlayer audioPreview;
    private View emptyPreview;
    private TextView selectedClip;
    private LinearLayout videoTrackContent;
    private LinearLayout audioTrackContent;
    private final ExecutorService mediaExecutor = Executors.newSingleThreadExecutor();
    private int selectedClipId = -1;
    private long selectedClipDurationMs = 0;
    private long selectedSourceInMs = 0;
    private long selectedSourceOutMs = -1;
    private int selectedTransitionType = 0;
    private long selectedTransitionDurationMs = 500;
    private String selectedClipSourceUri;
    private String selectedClipMimeType;
    private String selectedClipName;
    private MediaProbe.Info selectedClipInfo;
    private TextView playButton;
    private SeekBar previewSeekBar;
    private long previewDurationMs;
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

        AppContextHolder.initialize(getApplicationContext());
        mediaEngine = new NativeMediaEngine();
        engineStatus = mediaEngine.initialize();
        pluginHost = new PluginHost(this);
        audioPreview = new AudioPreviewPlayer(this, new AudioPreviewPlayer.Listener() {
            @Override public void onPlaybackChanged(boolean isPlaying) {
                runOnUiThread(() -> {
                    playing = isPlaying;
                    if (playButton != null) playButton.setText(isPlaying ? "Ⅱ" : "▶");
                });
            }
            @Override public void onError(String message) { runOnUiThread(() -> toast(message)); }
        });
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
        content.addView(buildPreviewControls(), marginParams(64, 8, 0, 14));
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
        projectTitle = label(ProjectStore.getProjectName(getApplicationContext()).toUpperCase(Locale.US), 9, MUTED, true);
        names.addView(projectTitle);
        names.setOnClickListener(v -> showRenameProjectDialog());
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

        int glVersion = 2;
        videoPreview = new GlVideoPreview(this);
        videoPreview.setOnPlaybackEnded(() -> {
            playing = false;
            if (playButton != null) playButton.setText("▶");
        });
        GLSurfaceView glView = videoPreview.getView();
        frame.addView(glView, new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));

        LinearLayout topLabel = new LinearLayout(this);
        topLabel.setGravity(Gravity.CENTER_VERTICAL);
        topLabel.setPadding(dp(10), dp(6), dp(10), dp(6));
        topLabel.setBackground(round(0xAA171821, 20));
        TextView live = label("●  PREVIEW", 10, 0xFFC5F7D3, true);
        topLabel.addView(live);
        FrameLayout.LayoutParams badgeParams = new FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, dp(28), Gravity.TOP | Gravity.LEFT);
        badgeParams.setMargins(dp(10), dp(10), 0, 0);
        frame.addView(topLabel, badgeParams);

        LinearLayout empty = new LinearLayout(this);
        empty.setOrientation(LinearLayout.VERTICAL);
        empty.setGravity(Gravity.CENTER);
        TextView icon = label("▶", 30, 0xFF6F8B78, true);
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
        LinearLayout container = new LinearLayout(this);
        container.setOrientation(LinearLayout.VERTICAL);
        LinearLayout row = new LinearLayout(this);
        row.setGravity(Gravity.CENTER_VERTICAL);
        TextView time = label("00:00:00", 12, MUTED, true);
        row.addView(time, new LinearLayout.LayoutParams(0, dp(36), 1f));

        playButton = action("▶", PANEL_ALT, TEXT);
        playButton.setGravity(Gravity.CENTER);
        playButton.setOnClickListener(v -> {
            if (audioPreview != null && audioPreview.hasAudio()) {
                audioPreview.toggle();
                playing = audioPreview.isPlaying();
            } else if (videoPreview != null && videoPreview.hasVideo()) {
                videoPreview.togglePlayback();
                playing = videoPreview.isPlaying();
            } else {
                toast("Import video or audio to start playback.");
                return;
            }
            playButton.setText(playing ? "Ⅱ" : "▶");
        });
        row.addView(playButton, new LinearLayout.LayoutParams(dp(42), dp(36)));

        TextView duration = label("00:00:00", 12, MUTED, true);
        duration.setGravity(Gravity.RIGHT | Gravity.CENTER_VERTICAL);
        row.addView(duration, new LinearLayout.LayoutParams(0, dp(36), 1f));
        container.addView(row, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(36)));

        previewSeekBar = new SeekBar(this);
        previewSeekBar.setMax(1000);
        previewSeekBar.setProgress(0);
        previewSeekBar.setEnabled(false);
        previewSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) { }
            @Override public void onStartTrackingTouch(SeekBar seekBar) { }
            @Override public void onStopTrackingTouch(SeekBar seekBar) {
                if (previewDurationMs > 0 && videoPreview != null && videoPreview.hasVideo()) {
                    long targetMs = previewDurationMs * seekBar.getProgress() / seekBar.getMax();
                    videoPreview.seekTo(targetMs);
                }
            }
        });
        container.addView(previewSeekBar, new LinearLayout.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT, dp(24)));
        return container;
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
        panel.addView(track("V1", "＋  Import video or image", 0xFF244B32, videoTrackContent),
                marginParams(ViewGroup.LayoutParams.MATCH_PARENT, 48, 0, 7));
        audioTrackContent = new LinearLayout(this);
        audioTrackContent.setGravity(Gravity.CENTER_VERTICAL);
        panel.addView(track("A1", "＋  Import audio", 0xFF244B32, audioTrackContent),
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
        chip.setBackground(round(audio ? 0xFF31545A : 0xFF245B39, 8));
        chip.setTag(clipId);
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(dp(124), dp(38));
        params.setMargins(0, 0, dp(6), 0);
        target.addView(chip, params);
        chip.setOnClickListener(v -> {
            selectedClipId = clipId;
            selectedClipDurationMs = info.durationMs;
            selectedClipSourceUri = sourceUri;
            selectedClipMimeType = mimeType;
            selectedClipName = title;
            selectedClipInfo = info;
            loadSelectedClipSettings(clipId);
            selectedClip.setText("Selected #" + clipId + " · " + title
                    + " · " + dimensions + " · " + duration);
            if (mimeType != null && mimeType.startsWith("video/")) {
                showVideoPreview(Uri.parse(sourceUri));
            } else if (mimeType != null && mimeType.startsWith("audio/")) {
                showAudioPreview(Uri.parse(sourceUri));
            }
        });
        selectedClipId = clipId;
        selectedClipDurationMs = info.durationMs;
        selectedClipSourceUri = sourceUri;
        selectedClipMimeType = mimeType;
        selectedClipName = title;
        selectedClipInfo = info;
        loadSelectedClipSettings(clipId);
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

    private void splitSelectedAtPlayhead() {
        if (selectedClipId < 0 || selectedClipInfo == null || selectedClipSourceUri == null) {
            toast("Select a timeline clip first.");
            return;
        }
        if (previewSeekBar == null || selectedClipDurationMs <= 0) {
            toast("This clip has no known duration.");
            return;
        }
        long splitAtMs = previewSeekBar.getProgress();
        if (splitAtMs <= 0 || splitAtMs >= selectedClipDurationMs) {
            toast("Move the playhead inside the selected clip before splitting.");
            return;
        }
        int newClipId = mediaEngine.splitClip(selectedClipId, splitAtMs);
        if (newClipId < 0) {
            toast("Could not split this clip at that position.");
            return;
        }
        addClipToTrack(newClipId, selectedClipName + " (split)",
                selectedClipMimeType, selectedClipInfo, selectedClipSourceUri);
        selectedClip.setText("Split clip #" + selectedClipId + " at "
                + formatTime(splitAtMs) + " · new clip #" + newClipId);
        toast("Clip split in the native timeline.");
    }

    private void showEffectsDialog() {
        if (videoPreview == null || !videoPreview.hasVideo()) {
            toast("Import a video before applying preview filters.");
            return;
        }
        String[] filters = {"Normal", "Monochrome", "Warm", "Cool", "Sepia", "Vintage", "Invert", "High contrast"};
        new AlertDialog.Builder(this).setTitle("Preview effects")
                .setSingleChoiceItems(filters, -1, (dialog, which) -> {
                    videoPreview.setFilterMode(which);
                    dialog.dismiss();
                    toast(filters[which] + " preview filter applied.");
                })
                .setNegativeButton("Cancel", null).show();
    }


    private void showFadeDialog() {
        if (videoPreview == null || !videoPreview.hasVideo()) {
            toast("Import a video before configuring fades.");
            return;
        }
        LinearLayout fields = new LinearLayout(this);
        fields.setOrientation(LinearLayout.VERTICAL);
        fields.setPadding(dp(20), dp(8), dp(20), 0);
        android.widget.CheckBox fadeIn = new android.widget.CheckBox(this);
        fadeIn.setText("Fade in from black");
        fadeIn.setTextColor(TEXT);
        android.widget.CheckBox fadeOut = new android.widget.CheckBox(this);
        fadeOut.setText("Fade out to black");
        fadeOut.setTextColor(TEXT);
        SeekBar duration = new SeekBar(this);
        duration.setMax(29);
        duration.setProgress(4);
        TextView durationLabel = label("Duration: 500 ms", 12, MUTED, false);
        duration.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int value, boolean fromUser) {
                durationLabel.setText("Duration: " + ((value + 1) * 100) + " ms");
            }
            @Override public void onStartTrackingTouch(SeekBar bar) { }
            @Override public void onStopTrackingTouch(SeekBar bar) { }
        });
        fields.addView(fadeIn);
        fields.addView(fadeOut);
        fields.addView(durationLabel);
        fields.addView(duration);
        new AlertDialog.Builder(this).setTitle("Clip fades")
                .setMessage("Preview fade transitions at the start and end of the selected video.")
                .setView(fields)
                .setNegativeButton("Cancel", null)
                .setPositiveButton("Apply", (dialog, which) -> {
                    int durationMs = (duration.getProgress() + 1) * 100;
                    videoPreview.setFadeSettings(fadeIn.isChecked(), fadeOut.isChecked(), durationMs);
                    toast("Fade settings applied to video preview.");
                }).show();
    }

    private void loadSelectedClipSettings(int clipId) {
        long[] settings = mediaEngine.getClipSettings(clipId);
        if (settings == null || settings.length < 4) return;
        selectedSourceInMs = settings[0];
        selectedSourceOutMs = settings[1];
        selectedTransitionType = (int) settings[2];
        selectedTransitionDurationMs = settings[3];
    }

    private void showTrimDialog() {
        if (selectedClipId < 0) { toast("Select a timeline clip first."); return; }
        if (selectedClipDurationMs <= 0) { toast("This source has no known duration for range trimming."); return; }
        int max = (int) Math.min(Integer.MAX_VALUE, selectedClipDurationMs);
        int initialStart = (int) Math.max(0, Math.min(max - 1, selectedSourceInMs));
        int initialEnd = (int) Math.max(initialStart + 1,
                Math.min(max, selectedSourceOutMs < 0 ? max : selectedSourceOutMs));
        LinearLayout fields = new LinearLayout(this);
        fields.setOrientation(LinearLayout.VERTICAL);
        fields.setPadding(dp(20), dp(8), dp(20), 0);
        TextView startLabel = label("", 12, TEXT, true);
        TextView endLabel = label("", 12, TEXT, true);
        SeekBar start = new SeekBar(this);
        SeekBar end = new SeekBar(this);
        start.setMax(Math.max(1, max - 1));
        end.setMax(max);
        start.setProgress(initialStart);
        end.setProgress(initialEnd);
        Runnable updateLabels = () -> {
            startLabel.setText("IN  ·  " + formatTime(start.getProgress()));
            endLabel.setText("OUT ·  " + formatTime(end.getProgress()));
        };
        start.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int value, boolean fromUser) {
                if (value >= end.getProgress()) end.setProgress(Math.min(max, value + 1));
                updateLabels.run();
            }
            @Override public void onStartTrackingTouch(SeekBar bar) { }
            @Override public void onStopTrackingTouch(SeekBar bar) { }
        });
        end.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int value, boolean fromUser) {
                if (value <= start.getProgress()) start.setProgress(Math.max(0, value - 1));
                updateLabels.run();
            }
            @Override public void onStartTrackingTouch(SeekBar bar) { }
            @Override public void onStopTrackingTouch(SeekBar bar) { }
        });
        updateLabels.run();
        fields.addView(startLabel);
        fields.addView(start);
        fields.addView(endLabel);
        fields.addView(end);
        new AlertDialog.Builder(this).setTitle("Custom clip trim")
                .setMessage("Drag the IN and OUT handles to choose the exact source range.")
                .setView(fields).setNegativeButton("Cancel", null)
                .setPositiveButton("Apply", (dialog, which) -> {
                    long inMs = start.getProgress();
                    long outMs = end.getProgress();
                    if (!mediaEngine.trimClip(selectedClipId, inMs, outMs)) {
                        toast("Invalid trim range.");
                        return;
                    }
                    selectedSourceInMs = inMs;
                    selectedSourceOutMs = outMs;
                    selectedClip.setText("Clip #" + selectedClipId + " · "
                            + formatTime(inMs) + " – " + formatTime(outMs));
                    toast("Custom trim saved in the C++ timeline.");
                }).show();
    }

    private void showTransitionDialog() {
        if (selectedClipId < 0) { toast("Select a timeline clip first."); return; }
        String[] types = {"None", "Fade", "Dissolve", "Wipe left → right",
                "Wipe right → left", "Slide left → right", "Slide right → left"};
        final int[] chosenType = {selectedTransitionType};
        LinearLayout fields = new LinearLayout(this);
        fields.setOrientation(LinearLayout.VERTICAL);
        fields.setPadding(dp(20), dp(8), dp(20), 0);
        android.widget.RadioGroup choices = new android.widget.RadioGroup(this);
        choices.setOrientation(android.widget.RadioGroup.VERTICAL);
        for (int i = 0; i < types.length; i++) {
            android.widget.RadioButton option = new android.widget.RadioButton(this);
            option.setText(types[i]);
            option.setTextColor(TEXT);
            option.setId(1000 + i);
            choices.addView(option);
        }
        choices.check(1000 + selectedTransitionType);
        choices.setOnCheckedChangeListener((group, checkedId) -> chosenType[0] = checkedId - 1000);
        TextView durationLabel = label("", 12, TEXT, true);
        SeekBar duration = new SeekBar(this);
        duration.setMax(58); // 100–3000 ms in 50 ms increments.
        int currentDuration = (int) Math.max(100, Math.min(3000, selectedTransitionDurationMs));
        duration.setProgress((currentDuration - 100) / 50);
        Runnable updateDuration = () -> durationLabel.setText(
                "Duration · " + (100 + duration.getProgress() * 50) + " ms");
        duration.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar bar, int value, boolean fromUser) { updateDuration.run(); }
            @Override public void onStartTrackingTouch(SeekBar bar) { }
            @Override public void onStopTrackingTouch(SeekBar bar) { }
        });
        updateDuration.run();
        fields.addView(choices);
        fields.addView(durationLabel);
        fields.addView(duration);
        new AlertDialog.Builder(this).setTitle("Custom clip transition")
                .setView(fields)
                .setNegativeButton("Cancel", null)
                .setPositiveButton("Apply", (dialog, which) -> {
                    int type = chosenType[0];
                    long durationMs = type == 0 ? 0 : 100 + duration.getProgress() * 50L;
                    long clipLength = selectedSourceOutMs >= 0
                            ? selectedSourceOutMs - selectedSourceInMs : selectedClipDurationMs;
                    if (type != 0 && clipLength > 0 && durationMs > clipLength) {
                        toast("Transition duration cannot exceed the trimmed clip length.");
                        return;
                    }
                    if (!mediaEngine.setClipTransition(selectedClipId, type, durationMs)) {
                        toast("Could not save this transition.");
                        return;
                    }
                    selectedTransitionType = type;
                    selectedTransitionDurationMs = durationMs;
                    selectedClip.setText("Clip #" + selectedClipId + " · " + types[type]
                            + (durationMs > 0 ? " · " + durationMs + " ms" : ""));
                    toast("Transition saved in the C++ timeline.");
                }).show();
    }

    private String formatPreciseTime(long milliseconds) {
        long safe = Math.max(0, milliseconds);
        return formatTime(safe) + String.format(Locale.US, ".%03d", safe % 1000);
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
        tools.addView(tool("⌁", "Split", false));
        tools.addView(tool("T", "Text", false));
        tools.addView(tool("◐", "Effects", false));
        tools.addView(tool("↗", "Fade", false));
        tools.addView(tool("⇄", "Transition", false));
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
        item.setBackground(round(primary ? 0xFF173B27 : PANEL, 12));
        TextView symbol = label(glyph, 19, primary ? 0xFFB7F7C8 : TEXT, true);
        symbol.setGravity(Gravity.CENTER);
        item.addView(symbol);
        TextView caption = label(name, 10, primary ? 0xFFE5D9FF : MUTED, true);
        item.addView(caption);
        item.setOnClickListener(v -> {
            if ("Media".equals(name)) {
                openMediaPicker();
            } else if ("Trim".equals(name)) {
                showTrimDialog();
            } else if ("Split".equals(name)) {
                splitSelectedAtPlayhead();
            } else if ("Effects".equals(name)) {
                showEffectsDialog();
            } else if ("Fade".equals(name)) {
                showFadeDialog();
            } else if ("Transition".equals(name)) {
                showTransitionDialog();
            } else if ("Audio".equals(name)) {
                openMediaPicker();
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
        TextView plugin = label("Developer plugins: JavaScript runtime available", 10, 0xFF86DDA0, false);
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
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
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
                if (info.durationMs > 0) mediaEngine.setClipDuration(clipId, info.durationMs);
                runOnUiThread(() -> {
                    if (isFinishing() || isDestroyed()) return;
                    addClipToTrack(clipId, source.displayName, source.mimeType, info, source.uri);
                    if (source.mimeType != null && source.mimeType.startsWith("video/")) {
                        showVideoPreview(uri);
                    } else if (source.mimeType != null && source.mimeType.startsWith("audio/")) {
                        showAudioPreview(uri);
                    } else if (frame != null) {
                        showPreviewFrame(frame);
                    }
                    mediaHint.setText("Project media restored");
                });
            }
        });
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_MEDIA || resultCode != RESULT_OK || data == null) return;

        ClipData selectedItems = data.getClipData();
        if (selectedItems != null) {
            for (int i = 0; i < selectedItems.getItemCount(); i++) {
                Uri itemUri = selectedItems.getItemAt(i).getUri();
                if (itemUri != null) importMediaUri(itemUri);
            }
        } else if (data.getData() != null) {
            importMediaUri(data.getData());
        }
    }

    private void importMediaUri(Uri uri) {
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
            if (clipId >= 0 && info.durationMs > 0) {
                mediaEngine.setClipDuration(clipId, info.durationMs);
            }
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
                if (mimeType != null && mimeType.startsWith("video/")) {
                    showVideoPreview(uri);
                } else if (mimeType != null && mimeType.startsWith("audio/")) {
                    showAudioPreview(uri);
                } else if (frame != null) {
                    showPreviewFrame(frame);
                }
                toast("Imported media · Clip #" + clipId);
            });
        });
    }

    private void showRenameProjectDialog() {
        EditText input = new EditText(this);
        input.setSingleLine(true);
        input.setText(ProjectStore.getProjectName(getApplicationContext()));
        input.setSelection(input.getText().length());
        input.setHint("Project name");
        new AlertDialog.Builder(this).setTitle("Rename project")
                .setView(input).setNegativeButton("Cancel", null)
                .setPositiveButton("Save", (dialog, which) -> {
                    String name = input.getText().toString().trim();
                    if (name.isEmpty()) {
                        toast("Project name cannot be empty.");
                        return;
                    }
                    ProjectStore.setProjectName(getApplicationContext(), name);
                    projectTitle.setText(name.toUpperCase(Locale.US));
                    toast("Project renamed.");
                }).show();
    }

    @Override
    protected void onDestroy() {
        mediaExecutor.shutdown();
        if (videoPreview != null) videoPreview.release();
        if (audioPreview != null) audioPreview.release();
        super.onDestroy();
    }

    private void showPreviewFrame(Bitmap bitmap) {
        if (bitmap == null || previewImage == null) return;
        if (audioPreview != null) audioPreview.release();
        previewDurationMs = 0;
        if (previewSeekBar != null) previewSeekBar.setEnabled(false);
        if (videoPreview != null) {
            videoPreview.clearVideo();
            videoPreview.getView().setVisibility(View.GONE);
        }
        previewImage.setImageBitmap(bitmap);
        previewImage.setVisibility(View.VISIBLE);
        if (emptyPreview != null) emptyPreview.setVisibility(View.GONE);
        if (playButton != null) playButton.setText("▶");
        playing = false;
    }

    private void showVideoPreview(Uri uri) {
        if (videoPreview == null || uri == null) return;
        if (audioPreview != null) audioPreview.release();
        previewDurationMs = selectedClipDurationMs;
        if (previewSeekBar != null) {
            previewSeekBar.setMax((int) Math.max(1, Math.min(Integer.MAX_VALUE, previewDurationMs)));
            previewSeekBar.setProgress(0);
            previewSeekBar.setEnabled(previewDurationMs > 0);
        }
        previewImage.setVisibility(View.GONE);
        if (emptyPreview != null) emptyPreview.setVisibility(View.GONE);
        videoPreview.getView().setVisibility(View.VISIBLE);
        java.util.ArrayList<Uri> videoSources = new java.util.ArrayList<>();
        for (ProjectStore.Source source : ProjectStore.load(getApplicationContext())) {
            if (source.mimeType != null && source.mimeType.startsWith("video/")) {
                videoSources.add(Uri.parse(source.uri));
            }
        }
        if (videoSources.isEmpty()) videoSources.add(uri);
        videoPreview.setTransitionSettings(selectedTransitionType, (int) selectedTransitionDurationMs);
        videoPreview.setPlaylist(videoSources, uri);
        if (playButton != null) playButton.setText("Ⅱ");
        playing = true;
    }

    private void showAudioPreview(Uri uri) {
        if (uri == null || audioPreview == null) return;
        previewDurationMs = 0;
        if (previewSeekBar != null) previewSeekBar.setEnabled(false);
        if (videoPreview != null) {
            videoPreview.clearVideo();
            videoPreview.getView().setVisibility(View.GONE);
        }
        if (previewImage != null) previewImage.setVisibility(View.GONE);
        if (emptyPreview != null) {
            emptyPreview.setVisibility(View.VISIBLE);
            mediaHint.setText("Audio preview · press play");
        }
        audioPreview.setSource(uri);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (videoPreview != null) videoPreview.onResume();
    }

    @Override
    protected void onPause() {
        if (videoPreview != null) videoPreview.onPause();
        if (audioPreview != null) audioPreview.pause();
        super.onPause();
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

}
