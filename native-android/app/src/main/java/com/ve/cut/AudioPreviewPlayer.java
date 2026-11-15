package com.ve.cut;

import android.content.Context;
import android.media.MediaPlayer;
import android.net.Uri;

/** Lightweight audio-track preview for imported audio sources. */
public final class AudioPreviewPlayer {
    public interface Listener {
        void onPlaybackChanged(boolean playing);
        void onError(String message);
    }

    private final Context context;
    private final Listener listener;
    private MediaPlayer player;
    private boolean prepared;
    private boolean wantsPlayback;

    public AudioPreviewPlayer(Context context, Listener listener) {
        this.context = context.getApplicationContext();
        this.listener = listener;
    }

    public synchronized void setSource(Uri uri) {
        releasePlayer();
        if (uri == null) return;
        wantsPlayback = true;
        MediaPlayer next = new MediaPlayer();
        player = next;
        try {
            next.setDataSource(context, uri);
            next.setOnPreparedListener(mp -> {
                synchronized (AudioPreviewPlayer.this) {
                    if (player != mp) return;
                    prepared = true;
                    if (wantsPlayback) {
                        mp.start();
                        listener.onPlaybackChanged(true);
                    }
                }
            });
            next.setOnCompletionListener(mp -> {
                synchronized (AudioPreviewPlayer.this) {
                    if (player == mp) {
                        wantsPlayback = false;
                        listener.onPlaybackChanged(false);
                    }
                }
            });
            next.setOnErrorListener((mp, what, extra) -> {
                synchronized (AudioPreviewPlayer.this) {
                    if (player == mp) {
                        releasePlayer();
                        listener.onPlaybackChanged(false);
                        listener.onError("Could not play this audio file.");
                    }
                }
                return true;
            });
            next.prepareAsync();
        } catch (Exception error) {
            releasePlayer();
            listener.onError(error.getMessage() == null
                    ? "Could not open this audio file." : error.getMessage());
        }
    }

    public synchronized void toggle() {
        if (player == null) return;
        if (!prepared) {
            wantsPlayback = !wantsPlayback;
            return;
        }
        if (player.isPlaying()) {
            player.pause();
            wantsPlayback = false;
            listener.onPlaybackChanged(false);
        } else {
            player.start();
            wantsPlayback = true;
            listener.onPlaybackChanged(true);
        }
    }

    public synchronized boolean hasAudio() { return player != null; }

    public synchronized boolean isPlaying() {
        return player != null && prepared && player.isPlaying();
    }

    public synchronized void pause() {
        if (player != null && prepared && player.isPlaying()) {
            player.pause();
            wantsPlayback = false;
            listener.onPlaybackChanged(false);
        }
    }

    public synchronized void release() { releasePlayer(); }

    private void releasePlayer() {
        MediaPlayer old = player;
        player = null;
        prepared = false;
        wantsPlayback = false;
        if (old != null) {
            old.setOnPreparedListener(null);
            old.setOnCompletionListener(null);
            old.setOnErrorListener(null);
            try { old.stop(); } catch (IllegalStateException ignored) { }
            old.release();
        }
    }
}
