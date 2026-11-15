package com.ve.cut;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import java.util.List;

public final class HomeActivity extends Activity {
    private static final int BG = Color.rgb(15, 16, 21);
    private static final int PANEL = Color.rgb(25, 29, 32);
    private static final int TEXT = Color.rgb(244, 247, 245);
    private static final int MUTED = Color.rgb(151, 165, 156);
    private static final int GREEN = Color.rgb(34, 197, 94);

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().setStatusBarColor(BG);
        getWindow().setNavigationBarColor(BG);
        setContentView(buildHome());
    }

    private View buildHome() {
        LinearLayout page = new LinearLayout(this);
        page.setOrientation(LinearLayout.VERTICAL);
        page.setBackgroundColor(BG);

        LinearLayout top = new LinearLayout(this);
        top.setGravity(Gravity.CENTER_VERTICAL);
        top.setPadding(dp(20), dp(18), dp(20), dp(14));
        TextView mark = text("V", 21, Color.WHITE, true);
        mark.setGravity(Gravity.CENTER);
        mark.setBackground(round(GREEN, 13));
        top.addView(mark, new LinearLayout.LayoutParams(dp(44), dp(44)));

        LinearLayout brand = new LinearLayout(this);
        brand.setOrientation(LinearLayout.VERTICAL);
        brand.setPadding(dp(11), 0, 0, 0);
        brand.addView(text("VeCut", 21, TEXT, true));
        brand.addView(text("VIDEO EDITOR", 9, MUTED, true));
        top.addView(brand, new LinearLayout.LayoutParams(0,
                ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        TextView settings = text("⚙", 20, TEXT, true);
        settings.setGravity(Gravity.CENTER);
        settings.setBackground(round(PANEL, 12));
        top.addView(settings, new LinearLayout.LayoutParams(dp(42), dp(42)));
        settings.setOnClickListener(v -> android.widget.Toast.makeText(this,
                "Settings are coming soon.", android.widget.Toast.LENGTH_SHORT).show());
        page.addView(top);

        ScrollView scroll = new ScrollView(this);
        LinearLayout content = new LinearLayout(this);
        content.setOrientation(LinearLayout.VERTICAL);
        content.setPadding(dp(20), dp(22), dp(20), dp(24));

        content.addView(text("Your next\ngreat edit.", 34, TEXT, true));
        TextView subtitle = text("Everything you need to make it yours.", 13, MUTED, false);
        content.addView(subtitle, params(-1, -2, 8, 24));

        LinearLayout create = new LinearLayout(this);
        create.setOrientation(LinearLayout.VERTICAL);
        create.setPadding(dp(18), dp(18), dp(18), dp(18));
        create.setBackground(round(0xFF123522, 18));
        create.addView(text("✦  START CREATING", 10, 0xFF9BEAB3, true));
        TextView title = text("New project", 22, Color.WHITE, true);
        create.addView(title, params(-1, -2, 10, 4));
        create.addView(text("Bring your videos, photos and audio together.", 12,
                0xFFC5E8D0, false));
        TextView button = text("＋   Create project", 14, Color.WHITE, true);
        button.setGravity(Gravity.CENTER);
        button.setBackground(round(GREEN, 12));
        create.addView(button, params(-1, 48, 17, 0));
        create.setOnClickListener(v -> openEditor());
        button.setOnClickListener(v -> openEditor());
        content.addView(create, params(-1, -2, 0, 28));

        LinearLayout heading = new LinearLayout(this);
        heading.setGravity(Gravity.CENTER_VERTICAL);
        heading.addView(text("Recent project", 17, TEXT, true),
                new LinearLayout.LayoutParams(0, -2, 1f));
        heading.addView(text("LOCAL", 9, GREEN, true));
        content.addView(heading, params(-1, 30, 0, 10));

        List<ProjectStore.Source> sources = ProjectStore.load(getApplicationContext());
        LinearLayout recent = new LinearLayout(this);
        recent.setGravity(Gravity.CENTER_VERTICAL);
        recent.setPadding(dp(13), dp(13), dp(13), dp(13));
        recent.setBackground(round(PANEL, 15));
        TextView thumbnail = text("▶", 22, 0xFF9BEAB3, true);
        thumbnail.setGravity(Gravity.CENTER);
        thumbnail.setBackground(round(0xFF1D3A2A, 11));
        recent.addView(thumbnail, new LinearLayout.LayoutParams(dp(62), dp(62)));
        LinearLayout details = new LinearLayout(this);
        details.setOrientation(LinearLayout.VERTICAL);
        details.setPadding(dp(12), 0, 0, 0);
        details.addView(text("Untitled project", 14, TEXT, true));
        details.addView(text(sources.isEmpty() ? "No imported media yet"
                : sources.size() + " media item(s) saved", 11, MUTED, false));
        recent.addView(details, new LinearLayout.LayoutParams(0, -2, 1f));
        recent.addView(text("›", 25, MUTED, true));
        recent.setOnClickListener(v -> openEditor());
        content.addView(recent);

        content.addView(text("QUICK TOOLS", 10, MUTED, true), params(-1, 20, 0, 10));
        LinearLayout tools = new LinearLayout(this);
        tools.addView(quickTool("✂", "Trim"));
        tools.addView(quickTool("T", "Text"));
        tools.addView(quickTool("♫", "Audio"));
        content.addView(tools);

        scroll.addView(content);
        page.addView(scroll, new LinearLayout.LayoutParams(-1, 0, 1f));
        TextView footer = text("VECUT  •  MADE FOR CREATORS", 9, MUTED, true);
        footer.setGravity(Gravity.CENTER);
        footer.setPadding(0, dp(12), 0, dp(16));
        page.addView(footer);
        return page;
    }

    private View quickTool(String symbol, String name) {
        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setGravity(Gravity.CENTER);
        card.setBackground(round(PANEL, 12));
        card.addView(text(symbol, 20, GREEN, true));
        card.addView(text(name, 11, TEXT, true), params(-2, -2, 0, 5));
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(0, dp(76), 1f);
        p.setMargins(0, 0, dp(8), 0);
        card.setLayoutParams(p);
        card.setOnClickListener(v -> openEditor());
        return card;
    }

    private void openEditor() {
        startActivity(new Intent(this, MainActivity.class));
    }

    private TextView text(String value, float size, int color, boolean bold) {
        TextView view = new TextView(this);
        view.setText(value);
        view.setTextSize(size);
        view.setTextColor(color);
        view.setIncludeFontPadding(false);
        if (bold) view.setTypeface(Typeface.DEFAULT, Typeface.BOLD);
        return view;
    }

    private GradientDrawable round(int color, int radius) {
        GradientDrawable shape = new GradientDrawable();
        shape.setColor(color);
        shape.setCornerRadius(dp(radius));
        return shape;
    }

    private LinearLayout.LayoutParams params(int width, int height, int top, int bottom) {
        LinearLayout.LayoutParams p = new LinearLayout.LayoutParams(width, height);
        p.topMargin = dp(top);
        p.bottomMargin = dp(bottom);
        return p;
    }

    private int dp(int value) {
        return Math.round(value * getResources().getDisplayMetrics().density);
    }
}
