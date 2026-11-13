package com.ve.cut;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

public final class MainActivity extends Activity {
    private TextView status;
    private NativeMediaEngine mediaEngine;
    private PluginHost pluginHost;

    @Override public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setStatusBarColor(0xFF171717);
        getWindow().setNavigationBarColor(0xFF171717);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER_HORIZONTAL);
        root.setPadding(28, 48, 28, 28);
        root.setBackgroundColor(0xFF202020);

        TextView title = new TextView(this);
        title.setText("VeCut");
        title.setTextColor(0xFFFFFFFF);
        title.setTextSize(32);
        root.addView(title, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        TextView subtitle = new TextView(this);
        subtitle.setText("Native Android editor · Java + C++ · JavaScript plugins");
        subtitle.setTextColor(0xFFBBBBBB);
        subtitle.setTextSize(14);
        LinearLayout.LayoutParams subtitleParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        subtitleParams.topMargin = 12;
        root.addView(subtitle, subtitleParams);

        status = new TextView(this);
        status.setTextColor(0xFFFFFFFF);
        status.setTextSize(14);
        status.setText("Starting media engine…");
        LinearLayout.LayoutParams statusParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        statusParams.topMargin = 32;
        root.addView(status, statusParams);

        Button loadPlugin = new Button(this);
        loadPlugin.setText("Load sample developer plugin");
        loadPlugin.setOnClickListener(view -> loadSamplePlugin());
        LinearLayout.LayoutParams buttonParams = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        buttonParams.topMargin = 20;
        root.addView(loadPlugin, buttonParams);

        setContentView(root);
        mediaEngine = new NativeMediaEngine();
        pluginHost = new PluginHost(this);
        status.setText("Media engine: " + mediaEngine.initialize() + "\nPlugin runtime: JavaScript ready");
    }

    private void loadSamplePlugin() {
        try {
            int count = pluginHost.loadAssetPlugin("plugins/sample-effect/plugin.js");
            status.setText("Sample plugin loaded. Registered effects: " + count + "\n" + pluginHost.getRegisteredEffectNames());
        } catch (Exception error) {
            status.setText("Plugin load failed: " + error.getMessage());
        }
    }
}
