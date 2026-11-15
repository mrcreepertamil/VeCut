package com.ve.cut;

import android.content.ContentResolver;
import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.net.Uri;
import android.view.Surface;

import java.io.IOException;
import java.nio.ByteBuffer;

/** Surface-output MediaCodec decoder with presentation-time scheduling. */
public final class CodecVideoPlayer {
    public interface Listener {
        void onPrepared();
        void onCompleted();
        void onError(String message);
    }

    private volatile boolean stopRequested;
    private volatile boolean paused;
    private volatile boolean playing;
    private volatile boolean finished;
    private Thread worker;

    public synchronized void play(Uri uri, Surface surface, Listener listener) {
        play(uri, surface, listener, 0);
    }

    public synchronized void play(Uri uri, Surface surface, Listener listener, long seekPositionMs) {
        stop();
        if (uri == null || surface == null || !surface.isValid()) {
            listener.onError("Video preview surface is not ready.");
            return;
        }
        stopRequested = false;
        finished = false;
        paused = false;
        playing = true;
        long safeSeekMs = Math.max(0, seekPositionMs);
        worker = new Thread(() -> decode(uri, surface, listener, safeSeekMs),
                "VeCut-MediaCodec");
        worker.start();
    }

    public void pause() {
        paused = true;
        playing = false;
    }

    public void resume() {
        if (worker != null && worker.isAlive()) {
            paused = false;
            playing = true;
        }
    }

    public boolean isPlaying() { return playing; }

    public boolean canResume() {
        Thread current = worker;
        return current != null && current.isAlive() && !stopRequested && !finished;
    }

    public synchronized void stop() {
        stopRequested = true;
        paused = false;
        playing = false;
        Thread old = worker;
        worker = null;
        if (old != null && old != Thread.currentThread()) {
            old.interrupt();
            try { old.join(700); }
            catch (InterruptedException error) { Thread.currentThread().interrupt(); }
        }
    }

    private void decode(Uri uri, Surface surface, Listener listener, long seekPositionMs) {
        MediaExtractor extractor = new MediaExtractor();
        MediaCodec codec = null;
        try {
            ContentResolver resolver = AppContextHolder.getResolver();
            if (resolver == null) throw new IOException("Application media resolver unavailable.");
            try (android.content.res.AssetFileDescriptor afd =
                         resolver.openAssetFileDescriptor(uri, "r")) {
                if (afd == null) throw new IOException("Could not open selected video.");
                long sourceLength = afd.getLength();
                if (sourceLength >= 0) {
                    extractor.setDataSource(afd.getFileDescriptor(),
                            afd.getStartOffset(), sourceLength);
                } else if (afd.getStartOffset() == 0) {
                    extractor.setDataSource(afd.getFileDescriptor());
                } else {
                    throw new IOException("This document provider does not expose the video length.");
                }
            }

            int videoTrack = -1;
            MediaFormat videoFormat = null;
            for (int i = 0; i < extractor.getTrackCount(); i++) {
                MediaFormat format = extractor.getTrackFormat(i);
                String mime = format.getString(MediaFormat.KEY_MIME);
                if (mime != null && mime.startsWith("video/")) {
                    videoTrack = i;
                    videoFormat = format;
                    break;
                }
            }
            if (videoTrack < 0 || videoFormat == null) {
                throw new IOException("This file does not contain a video track.");
            }
            extractor.selectTrack(videoTrack);
            long requestedPositionUs = seekPositionMs * 1000L;
            if (requestedPositionUs > 0) {
                extractor.seekTo(requestedPositionUs, MediaExtractor.SEEK_TO_PREVIOUS_SYNC);
            }
            String mime = videoFormat.getString(MediaFormat.KEY_MIME);
            codec = MediaCodec.createDecoderByType(mime);
            codec.configure(videoFormat, surface, null, 0);
            codec.start();
            listener.onPrepared();

            MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
            boolean inputDone = false;
            boolean outputDone = false;
            long firstPresentationUs = -1;
            long playbackStartNs = System.nanoTime();

            while (!outputDone && !stopRequested) {
                if (paused) {
                    try { Thread.sleep(8); }
                    catch (InterruptedException ignored) { }
                    continue;
                }

                if (!inputDone) {
                    int inputIndex = codec.dequeueInputBuffer(10000);
                    if (inputIndex >= 0) {
                        ByteBuffer input = codec.getInputBuffer(inputIndex);
                        if (input == null) throw new IOException("Decoder input buffer unavailable.");
                        int sampleSize = extractor.readSampleData(input, 0);
                        if (sampleSize < 0) {
                            codec.queueInputBuffer(inputIndex, 0, 0, 0,
                                    MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                            inputDone = true;
                        } else {
                            long presentationUs = extractor.getSampleTime();
                            codec.queueInputBuffer(inputIndex, 0, sampleSize,
                                    presentationUs, extractor.getSampleFlags());
                            extractor.advance();
                        }
                    }
                }

                int outputIndex = codec.dequeueOutputBuffer(info, 10000);
                if (outputIndex >= 0) {
                    boolean endOfStream =
                            (info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0;
                    if (info.size > 0) {
                        if (info.presentationTimeUs < requestedPositionUs) {
                            codec.releaseOutputBuffer(outputIndex, false);
                        } else {
                            if (firstPresentationUs < 0) {
                                firstPresentationUs = info.presentationTimeUs;
                                playbackStartNs = System.nanoTime();
                            }
                            long renderAtNs = playbackStartNs
                                    + Math.max(0, info.presentationTimeUs - firstPresentationUs) * 1000L;
                            codec.releaseOutputBuffer(outputIndex, renderAtNs);
                        }
                    } else {
                        codec.releaseOutputBuffer(outputIndex, false);
                    }
                    if (endOfStream) outputDone = true;
                }
            }
            playing = false;
            finished = true;
            if (!stopRequested) listener.onCompleted();
        } catch (Exception error) {
            playing = false;
            finished = true;
            if (!stopRequested) listener.onError(error.getMessage() == null
                    ? error.getClass().getSimpleName() : error.getMessage());
        } finally {
            playing = false;
            if (codec != null) {
                try { codec.stop(); } catch (Exception ignored) { }
                try { codec.release(); } catch (Exception ignored) { }
            }
            extractor.release();
        }
    }
}
