package com.ve.cut;

import android.content.Context;
import android.util.Log;

import org.mozilla.javascript.BaseFunction;
import org.mozilla.javascript.ContextFactory;
import org.mozilla.javascript.EvaluatorException;
import org.mozilla.javascript.Scriptable;
import org.mozilla.javascript.ScriptableObject;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/** Executes JavaScript plugins with a deliberately small host API and no Java class access. */
public final class PluginHost {
    private static final String TAG = "VeCutPlugins";
    private static final int INSTRUCTION_LIMIT = 250_000;
    private final Context appContext;
    private final List<String> effects = new ArrayList<>();

    private final ContextFactory sandboxFactory = new ContextFactory() {
        @Override protected void observeInstructionCount(org.mozilla.javascript.Context cx, int instructionCount) {
            throw new EvaluatorException("Plugin exceeded its execution budget");
        }
    };

    public PluginHost(Context context) { appContext = context.getApplicationContext(); }

    public int loadAssetPlugin(String assetPath) throws Exception {
        StringBuilder source = new StringBuilder();
        try (BufferedReader reader = new BufferedReader(new InputStreamReader(appContext.getAssets().open(assetPath), StandardCharsets.UTF_8))) {
            String line;
            while ((line = reader.readLine()) != null) source.append(line).append('\n');
        }

        org.mozilla.javascript.Context cx = sandboxFactory.enterContext();
        try {
            cx.setOptimizationLevel(-1);
            cx.setLanguageVersion(org.mozilla.javascript.Context.VERSION_ES6);
            cx.setInstructionObserverThreshold(INSTRUCTION_LIMIT);
            cx.setClassShutter(className -> false);
            Scriptable scope = cx.initSafeStandardObjects();
            Scriptable api = cx.newObject(scope);

            ScriptableObject.putProperty(api, "log", new BaseFunction(scope, ScriptableObject.getFunctionPrototype(scope)) {
                @Override public Object call(org.mozilla.javascript.Context context, Scriptable callScope, Scriptable thisObj, Object[] args) {
                    Log.i(TAG, args.length == 0 ? "Plugin log" : String.valueOf(args[0]));
                    return org.mozilla.javascript.Undefined.instance;
                }
            });
            ScriptableObject.putProperty(api, "registerEffect", new BaseFunction(scope, ScriptableObject.getFunctionPrototype(scope)) {
                @Override public Object call(org.mozilla.javascript.Context context, Scriptable callScope, Scriptable thisObj, Object[] args) {
                    if (args.length == 0 || !(args[0] instanceof Scriptable)) throw new EvaluatorException("registerEffect expects an effect descriptor");
                    Scriptable descriptor = (Scriptable) args[0];
                    String id = ScriptableObject.getProperty(descriptor, "id") instanceof String ? (String) ScriptableObject.getProperty(descriptor, "id") : "";
                    String name = ScriptableObject.getProperty(descriptor, "name") instanceof String ? (String) ScriptableObject.getProperty(descriptor, "name") : "";
                    if (id.isEmpty() || name.isEmpty() || id.length() > 64 || name.length() > 80) throw new EvaluatorException("Effect needs a short id and name");
                    effects.add(name + " (" + id + ")");
                    return org.mozilla.javascript.Undefined.instance;
                }
            });
            ScriptableObject.putProperty(scope, "veCut", api);
            cx.evaluateString(scope, source.toString(), assetPath, 1, null);
            return effects.size();
        } finally {
            org.mozilla.javascript.Context.exit();
        }
    }

    public List<String> getRegisteredEffectNames() { return Collections.unmodifiableList(new ArrayList<>(effects)); }
}
