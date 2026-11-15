package com.ve.cut;

import android.content.Context;
import android.content.SharedPreferences;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

/** Persists imported source references for the current local editing project. */
public final class ProjectStore {
    private static final String PREFS = "vecut_project";
    private static final String KEY_SOURCES = "sources";
    private static final String KEY_PROJECT_NAME = "project_name";

    private ProjectStore() {}

    public static final class Source {
        public final String uri;
        public final String mimeType;
        public final String displayName;

        Source(String uri, String mimeType, String displayName) {
            this.uri = uri;
            this.mimeType = mimeType;
            this.displayName = displayName;
        }
    }

    public static synchronized List<Source> load(Context context) {
        String encoded = preferences(context).getString(KEY_SOURCES, "[]");
        ArrayList<Source> sources = new ArrayList<>();
        try {
            JSONArray array = new JSONArray(encoded);
            for (int i = 0; i < array.length(); i++) {
                JSONObject item = array.optJSONObject(i);
                if (item == null) continue;
                String uri = item.optString("uri", "");
                if (uri.isEmpty()) continue;
                sources.add(new Source(uri, item.optString("mimeType", ""),
                        item.optString("displayName", "Media file")));
            }
        } catch (JSONException ignored) {
            // A damaged project index is treated as an empty project.
        }
        return sources;
    }

    public static synchronized void add(Context context, String uri,
                                        String mimeType, String displayName) {
        List<Source> existing = load(context);
        existing.add(new Source(uri, mimeType == null ? "" : mimeType,
                displayName == null ? "Media file" : displayName));
        write(context, existing);
    }

    public static synchronized void remove(Context context, String uri) {
        List<Source> existing = load(context);
        for (int i = 0; i < existing.size(); i++) {
            if (existing.get(i).uri.equals(uri)) {
                existing.remove(i);
                break;
            }
        }
        write(context, existing);
    }

    public static synchronized String getProjectName(Context context) {\n        return preferences(context).getString(KEY_PROJECT_NAME, "Untitled project");\n    }\n\n    public static synchronized void setProjectName(Context context, String name) {\n        if (name == null || name.trim().isEmpty()) return;\n        preferences(context).edit().putString(KEY_PROJECT_NAME, name.trim()).apply();\n    }\n\n    public static synchronized void clear(Context context) {
        preferences(context).edit().remove(KEY_SOURCES).apply();
    }

    private static void write(Context context, List<Source> sources) {
        JSONArray array = new JSONArray();
        for (Source source : sources) {
            JSONObject item = new JSONObject();
            try {
                item.put("uri", source.uri);
                item.put("mimeType", source.mimeType);
                item.put("displayName", source.displayName);
                array.put(item);
            } catch (JSONException ignored) {
                // These fixed string fields should not fail JSON encoding.
            }
        }
        preferences(context).edit().putString(KEY_SOURCES, array.toString()).apply();
    }

    private static SharedPreferences preferences(Context context) {
        return context.getApplicationContext()
                .getSharedPreferences(PREFS, Context.MODE_PRIVATE);
    }
}
