package com.ve.cut;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Handler;
import android.os.Looper;
import android.view.Surface;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

/** OpenGL ES external-texture preview fed by a decoder Surface. */
public final class GlVideoPreview {
    private final GLSurfaceView view;
    private final CodecVideoPlayer player = new CodecVideoPlayer();
    private final Handler main = new Handler(Looper.getMainLooper());
    private volatile UriRequest pending;
    private Surface decoderSurface;
    private SurfaceTexture surfaceTexture;
    private boolean hasVideo;

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
        pending = new UriRequest(uri);
        main.post(() -> {
            if (decoderSurface != null) startPending();
        });
    }

    public boolean hasVideo() { return hasVideo; }

    public boolean isPlaying() { return player.isPlaying(); }

    public void togglePlayback() {
        if (!hasVideo) return;
        if (player.isPlaying()) player.pause();
        else player.resume();
    }

    public void release() {
        hasVideo = false;
        pending = null;
        player.stop();
        if (decoderSurface != null) {
            decoderSurface.release();
            decoderSurface = null;
        }
    }

    private void startPending() {
        UriRequest request = pending;
        if (request == null || decoderSurface == null) return;
        pending = null;
        player.play(request.uri, decoderSurface, new CodecVideoPlayer.Listener() {
            @Override public void onPrepared() { }
            @Override public void onCompleted() {
                main.post(() -> { if (hasVideo) player.pause(); });
            }
            @Override public void onError(String message) {
                main.post(() -> android.util.Log.e("VeCut", "Video decoder: " + message));
            }
        });
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
                if (decoderSurface != null) decoderSurface.release();
                surfaceTexture = texture;
                decoderSurface = new Surface(texture);
                startPending();
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
            "void main(){ gl_FragColor=texture2D(uTexture,vTexCoord); }";
}
