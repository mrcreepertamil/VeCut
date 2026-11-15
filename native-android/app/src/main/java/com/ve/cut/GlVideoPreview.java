package com.ve.cut;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.graphics.Rect;
import android.animation.ValueAnimator;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.Looper;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.List;

/** OpenGL ES external-texture preview fed by a decoder Surface. */
public final class GlVideoPreview {
    private final GLSurfaceView view;
    private final CodecVideoPlayer player = new CodecVideoPlayer();
    private final Handler main = new Handler(Looper.getMainLooper());
    private volatile UriRequest pending;
    private Surface decoderSurface;
    private volatile SurfaceTexture surfaceTexture;
    private volatile boolean hasVideo;
    private android.net.Uri lastUri;
    private final List<android.net.Uri> playlist = new ArrayList<>();
    private int playlistIndex;
    private boolean resumeAfterLifecyclePause;
    private Runnable playbackEndedCallback;
    private volatile int filterMode;
    private volatile boolean fadeInEnabled;
    private volatile boolean fadeOutEnabled;
    private volatile int fadeDurationMs = 500;
    private volatile int transitionType;
    private volatile int transitionDurationMs = 500;
    private volatile boolean transitionPending;
    private volatile long currentPositionMs;

    /** 0=normal, 1=monochrome, 2=warm, 3=cool. */
    public void setFilterMode(int mode) {
        filterMode = Math.max(0, Math.min(7, mode));
        view.requestRender();
    }

    /** Configure preview-only fades at the beginning and end of each decoded clip. */
    public void setTransitionSettings(int type, int durationMs) {
        transitionType = Math.max(0, Math.min(6, type));
        transitionDurationMs = Math.max(100, Math.min(3000, durationMs));
    }

    public void setFadeSettings(boolean fadeIn, boolean fadeOut, int durationMs) {
        fadeInEnabled = fadeIn;
        fadeOutEnabled = fadeOut;
        fadeDurationMs = Math.max(100, Math.min(3000, durationMs));
    }

    public GlVideoPreview(Context context) {
        view = new GLSurfaceView(context);
        view.setEGLContextClientVersion(2);
        view.setPreserveEGLContextOnPause(true);
        view.setRenderer(new PreviewRenderer());
        view.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
    }

    public GLSurfaceView getView() { return view; }

    public void setVideo(android.net.Uri uri) {
        if (uri == null) return;
        hasVideo = true;
        playlist.clear();
        playlist.add(uri);
        playlistIndex = 0;
        lastUri = uri;
        pending = new UriRequest(uri);
        main.post(() -> {
            if (decoderSurface != null) startPending();
        });
    }

    /** Plays imported video clips in the order supplied, beginning at startUri. */
    public void setPlaylist(List<android.net.Uri> uris, android.net.Uri startUri) {
        if (uris == null || uris.isEmpty()) {
            if (startUri != null) setVideo(startUri);
            return;
        }
        playlist.clear();
        playlist.addAll(uris);
        playlistIndex = Math.max(0, playlist.indexOf(startUri));
        lastUri = playlist.get(playlistIndex);
        hasVideo = true;
        pending = new UriRequest(lastUri);
        main.post(() -> {
            if (decoderSurface != null) startPending();
        });
    }

    public boolean hasVideo() { return hasVideo; }

    public boolean isPlaying() { return player.isPlaying(); }

    /** Restarts decoding from the nearest sync frame before the requested source time. */
    public void seekTo(long positionMs) {
        if (!hasVideo || lastUri == null || decoderSurface == null) return;
        currentPositionMs = Math.max(0, positionMs);
        startDecoder(lastUri, currentPositionMs);
    }

    public void setOnPlaybackEnded(Runnable callback) {
        playbackEndedCallback = callback;
    }

    public void togglePlayback() {
        if (!hasVideo) return;
        if (player.isPlaying()) {
            player.pause();
        } else if (player.canResume()) {
            player.resume();
        } else if (lastUri != null && decoderSurface != null) {
            startDecoder(lastUri);
        }
    }

    public void clearVideo() {
        hasVideo = false;
        pending = null;
        lastUri = null;
        playlist.clear();
        playlistIndex = 0;
        player.stop();
    }

    public void onResume() {
        view.onResume();
        if (resumeAfterLifecyclePause) {
            resumeAfterLifecyclePause = false;
            player.resume();
        }
    }

    public void onPause() {
        resumeAfterLifecyclePause = player.isPlaying();
        player.pause();
        view.onPause();
    }

    public void release() {
        clearVideo();
        if (decoderSurface != null) {
            decoderSurface.release();
            decoderSurface = null;
        }
        SurfaceTexture current = surfaceTexture;
        surfaceTexture = null;
        if (current != null) current.release();
    }

    private void startPending() {
        UriRequest request = pending;
        if (request == null || decoderSurface == null) return;
        pending = null;
        startDecoder(request.uri);
    }

    private void startDecoder(android.net.Uri uri) {
        currentPositionMs = 0;
        startDecoder(uri, 0);
    }

    private void startDecoder(android.net.Uri uri, long seekPositionMs) {
        main.post(() -> {
            if (!transitionPending) view.setAlpha(fadeInEnabled ? 0f : 1f);
        });
        player.play(uri, decoderSurface, new CodecVideoPlayer.Listener() {
            @Override public void onPrepared() {
                main.post(() -> {
                    if (transitionPending && transitionType > 0) {
                        playTransitionIn();
                    } else if (fadeInEnabled) {
                        view.animate().alpha(1f).setDuration(fadeDurationMs).start();
                    } else {
                        view.setAlpha(1f);
                    }
                    transitionPending = false;
                });
            }
            @Override public void onCompleted() {
                main.post(() -> {
                    Runnable finish = () -> {
                        if (playlistIndex + 1 < playlist.size() && hasVideo) {
                            playlistIndex++;
                            lastUri = playlist.get(playlistIndex);
                            startDecoder(lastUri);
                        } else if (playbackEndedCallback != null) {
                            playbackEndedCallback.run();
                        }
                    };
                    if (playlistIndex + 1 < playlist.size() && transitionType > 0) {
                        playTransitionOut(finish);
                    } else if (fadeOutEnabled) {
                        view.animate().alpha(0f).setDuration(fadeDurationMs)
                                .withEndAction(finish).start();
                    } else {
                        finish.run();
                    }
                });
            }
            @Override public void onError(String message) {
                main.post(() -> {
                    android.util.Log.e("VeCut", "Video decoder: " + message);
                    android.widget.Toast.makeText(view.getContext(),
                            "Video preview failed: " + message,
                            android.widget.Toast.LENGTH_LONG).show();
                    if (playbackEndedCallback != null) playbackEndedCallback.run();
                });
            }
        }, seekPositionMs);
    }

    private void playTransitionOut(Runnable finish) {
        transitionPending = true;
        final int width = Math.max(1, view.getWidth());
        final int height = Math.max(1, view.getHeight());
        switch (transitionType) {
            case 1: // Fade
            case 2: // Dissolve-style alpha blend
                view.animate().alpha(0f).setDuration(transitionDurationMs)
                        .withEndAction(finish).start();
                break;
            case 3:
            case 4: {
                final boolean leftToRight = transitionType == 3;
                ValueAnimator animator = ValueAnimator.ofFloat(0f, 1f);
                animator.setDuration(transitionDurationMs);
                animator.addUpdateListener(value -> {
                    float t = (float) value.getAnimatedValue();
                    int reveal = Math.max(0, Math.min(width, (int) (width * (1f - t))));
                    int left = leftToRight ? 0 : width - reveal;
                    view.setClipBounds(new Rect(left, 0, left + reveal, height));
                });
                animator.addListener(new android.animation.AnimatorListenerAdapter() {
                    @Override public void onAnimationEnd(android.animation.Animator animation) {
                        view.setClipBounds(null);
                        finish.run();
                    }
                });
                animator.start();
                break;
            }
            case 5:
            case 6: {
                float direction = transitionType == 5 ? -1f : 1f;
                view.animate().translationX(direction * width).setDuration(transitionDurationMs)
                        .withEndAction(finish).start();
                break;
            }
            default:
                finish.run();
        }
    }

    private void playTransitionIn() {
        final int width = Math.max(1, view.getWidth());
        final int height = Math.max(1, view.getHeight());
        switch (transitionType) {
            case 1:
            case 2:
                view.setAlpha(0f);
                view.animate().alpha(1f).setDuration(transitionDurationMs).start();
                break;
            case 3:
            case 4: {
                view.setAlpha(1f);
                final boolean leftToRight = transitionType == 3;
                view.setClipBounds(new Rect(leftToRight ? 0 : width, 0,
                        leftToRight ? 0 : width, height));
                ValueAnimator animator = ValueAnimator.ofFloat(0f, 1f);
                animator.setDuration(transitionDurationMs);
                animator.addUpdateListener(value -> {
                    float t = (float) value.getAnimatedValue();
                    int reveal = Math.max(0, Math.min(width, (int) (width * t)));
                    int left = leftToRight ? 0 : width - reveal;
                    view.setClipBounds(new Rect(left, 0, left + reveal, height));
                });
                animator.addListener(new android.animation.AnimatorListenerAdapter() {
                    @Override public void onAnimationEnd(android.animation.Animator animation) {
                        view.setClipBounds(null);
                    }
                });
                animator.start();
                break;
            }
            case 5:
            case 6:
                view.setAlpha(1f);
                view.setTranslationX((transitionType == 5 ? 1f : -1f) * width);
                view.animate().translationX(0f).setDuration(transitionDurationMs).start();
                break;
            default:
                view.setAlpha(1f);
        }
    }

    private final class PreviewRenderer implements GLSurfaceView.Renderer {
        private final float[] vertices = {
                -1f, -1f, 0f, 1f,
                 1f, -1f, 1f, 1f,
                -1f,  1f, 0f, 0f,
                 1f,  1f, 1f, 0f
        };
        private FloatBuffer vertexBuffer;
        private final float[] transform = new float[16];
        private int program;
        private int textureId;
        private int positionLocation;
        private int texCoordLocation;
        private int matrixLocation;
        private int samplerLocation;
        private int filterLocation;
        private SurfaceTexture texture;

        @Override public void onSurfaceCreated(javax.microedition.khronos.opengles.GL10 gl,
                                                javax.microedition.khronos.egl.EGLConfig config) {
            GLES20.glClearColor(0.035f, 0.038f, 0.052f, 1f);
            vertexBuffer = ByteBuffer.allocateDirect(vertices.length * 4)
                    .order(ByteOrder.nativeOrder()).asFloatBuffer();
            vertexBuffer.put(vertices).position(0);
            program = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
            positionLocation = GLES20.glGetAttribLocation(program, "aPosition");
            texCoordLocation = GLES20.glGetAttribLocation(program, "aTexCoord");
            matrixLocation = GLES20.glGetUniformLocation(program, "uTexMatrix");
            samplerLocation = GLES20.glGetUniformLocation(program, "uTexture");
            filterLocation = GLES20.glGetUniformLocation(program, "uFilter");
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            textureId = textures[0];
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                    GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                    GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                    GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                    GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

            texture = new SurfaceTexture(textureId);
            texture.setOnFrameAvailableListener(st -> view.requestRender());
            main.post(() -> {
                boolean replacingSurface = decoderSurface != null;
                if (replacingSurface) player.stop();
                if (decoderSurface != null) decoderSurface.release();
                SurfaceTexture previous = surfaceTexture;
                if (previous != null && previous != texture) previous.release();
                surfaceTexture = texture;
                decoderSurface = new Surface(texture);
                if (pending != null) startPending();
                else if (replacingSurface && hasVideo && lastUri != null) startDecoder(lastUri);
            });
        }

        @Override public void onSurfaceChanged(javax.microedition.khronos.opengles.GL10 gl,
                                               int width, int height) {
            GLES20.glViewport(0, 0, width, height);
        }

        @Override public void onDrawFrame(javax.microedition.khronos.opengles.GL10 gl) {
            GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
            SurfaceTexture current = surfaceTexture;
            if (current == null || program == 0) return;
            try {
                current.updateTexImage();
                current.getTransformMatrix(transform);
            } catch (RuntimeException ignored) {
                return;
            }
            GLES20.glUseProgram(program);
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId);
            GLES20.glUniform1i(samplerLocation, 0);
            GLES20.glUniform1i(filterLocation, filterMode);
            GLES20.glUniformMatrix4fv(matrixLocation, 1, false, transform, 0);
            vertexBuffer.position(0);
            GLES20.glEnableVertexAttribArray(positionLocation);
            GLES20.glVertexAttribPointer(positionLocation, 2, GLES20.GL_FLOAT,
                    false, 16, vertexBuffer);
            vertexBuffer.position(2);
            GLES20.glEnableVertexAttribArray(texCoordLocation);
            GLES20.glVertexAttribPointer(texCoordLocation, 2, GLES20.GL_FLOAT,
                    false, 16, vertexBuffer);
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
            GLES20.glDisableVertexAttribArray(positionLocation);
            GLES20.glDisableVertexAttribArray(texCoordLocation);
        }

        private int createProgram(String vertex, String fragment) {
            int vs = compileShader(GLES20.GL_VERTEX_SHADER, vertex);
            int fs = compileShader(GLES20.GL_FRAGMENT_SHADER, fragment);
            int result = GLES20.glCreateProgram();
            GLES20.glAttachShader(result, vs);
            GLES20.glAttachShader(result, fs);
            GLES20.glLinkProgram(result);
            int[] status = new int[1];
            GLES20.glGetProgramiv(result, GLES20.GL_LINK_STATUS, status, 0);
            if (status[0] == 0) {
                android.util.Log.e("VeCut", "GL link failed: " + GLES20.glGetProgramInfoLog(result));
                GLES20.glDeleteProgram(result);
                return 0;
            }
            GLES20.glDeleteShader(vs);
            GLES20.glDeleteShader(fs);
            return result;
        }

        private int compileShader(int type, String source) {
            int shader = GLES20.glCreateShader(type);
            GLES20.glShaderSource(shader, source);
            GLES20.glCompileShader(shader);
            int[] status = new int[1];
            GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, status, 0);
            if (status[0] == 0) {
                android.util.Log.e("VeCut", "GL shader failed: " + GLES20.glGetShaderInfoLog(shader));
                GLES20.glDeleteShader(shader);
                return 0;
            }
            return shader;
        }
    }

    private static final class UriRequest {
        final android.net.Uri uri;
        UriRequest(android.net.Uri uri) { this.uri = uri; }
    }

    private static final String VERTEX_SHADER =
            "attribute vec4 aPosition;\n" +
            "attribute vec2 aTexCoord;\n" +
            "uniform mat4 uTexMatrix;\n" +
            "varying vec2 vTexCoord;\n" +
            "void main(){ gl_Position=aPosition; vTexCoord=(uTexMatrix*vec4(aTexCoord,0.0,1.0)).xy; }";
    private static final String FRAGMENT_SHADER =
            "#extension GL_OES_EGL_image_external : require\n" +
            "precision mediump float;\n" +
            "uniform samplerExternalOES uTexture;\n" +
            "varying vec2 vTexCoord;\n" +
            "uniform int uFilter;\n" +
            "void main(){ vec4 c=texture2D(uTexture,vTexCoord); " +
            "if(uFilter==1){ float g=dot(c.rgb,vec3(0.299,0.587,0.114)); c.rgb=vec3(g); } " +
            "else if(uFilter==2){ c.r=min(1.0,c.r*1.08+0.025); c.g*=0.99; c.b*=0.90; } " +
            "else if(uFilter==3){ c.r*=0.90; c.g*=0.99; c.b=min(1.0,c.b*1.08+0.02); } " +
            "else if(uFilter==4){ vec3 s=vec3(dot(c.rgb,vec3(0.393,0.769,0.189)),dot(c.rgb,vec3(0.349,0.686,0.168)),dot(c.rgb,vec3(0.272,0.534,0.131))); c.rgb=min(s,vec3(1.0)); } " +
            "else if(uFilter==5){ float g=dot(c.rgb,vec3(0.299,0.587,0.114)); c.rgb=mix(vec3(g),c.rgb,0.55)*vec3(1.08,1.00,0.88); c.rgb=(c.rgb-0.5)*1.12+0.5; } " +
            "else if(uFilter==6){ c.rgb=vec3(1.0)-c.rgb; } " +
            "else if(uFilter==7){ c.rgb=clamp((c.rgb-0.5)*1.35+0.5,0.0,1.0); } " +
            "gl_FragColor=c; }";
}
