import 'dart:io';

import 'package:file_picker/file_picker.dart';
import 'package:flutter/material.dart';
import 'package:ffmpeg_kit_flutter_new/ffmpeg_kit.dart';
import 'package:ffmpeg_kit_flutter_new/return_code.dart';
import 'package:path_provider/path_provider.dart';
import 'package:video_player/video_player.dart';

import '../models/ve_cut_project.dart';

class EditorWorkspacePage extends StatefulWidget {
  const EditorWorkspacePage({super.key, required this.project});

  final VeCutProject project;

  @override
  State<EditorWorkspacePage> createState() => _EditorWorkspacePageState();
}

class _EditorWorkspacePageState extends State<EditorWorkspacePage> {
  VideoPlayerController? _controller;
  String? _videoPath;
  RangeValues _trimRange = const RangeValues(0, 1);
  bool _isExporting = false;
  String _status = 'Import a video to begin editing.';
  String? _exportedPath;
  static const int _freeTrackLimit = 12;
  final List<_TimelineTrack> _tracks = [
    const _TimelineTrack(label: 'Video', icon: Icons.movie_outlined),
  ];

  void _addTrack(String label, IconData icon) {
    if (_tracks.length >= _freeTrackLimit) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(content: Text('Free projects support up to 12 tracks.')),
      );
      return;
    }
    setState(() => _tracks.add(_TimelineTrack(label: label, icon: icon)));
  }

  Duration get _duration => _controller?.value.duration ?? Duration.zero;

  @override
  void dispose() {
    _controller?.dispose();
    super.dispose();
  }

  Future<void> _importVideo() async {
    final result = await FilePicker.platform.pickFiles(
      type: FileType.video,
      allowMultiple: false,
    );
    if (result == null || result.files.single.path == null) return;

    final path = result.files.single.path!;
    final nextController = VideoPlayerController.file(File(path));
    try {
      await nextController.initialize();
      await _controller?.dispose();
      if (!mounted) {
        await nextController.dispose();
        return;
      }
      setState(() {
        _controller = nextController;
        _videoPath = path;
        final milliseconds = nextController.value.duration.inMilliseconds;
        _trimRange = RangeValues(0, milliseconds.toDouble());
        _status = 'Video loaded. Adjust the trim handles, then export.';
        _exportedPath = null;
      });
    } catch (error) {
      await nextController.dispose();
      if (mounted) {
        setState(() => _status = 'Could not open this video: $error');
      }
    }
  }

  Future<void> _togglePlayback() async {
    final controller = _controller;
    if (controller == null || !controller.value.isInitialized) return;
    if (controller.value.isPlaying) {
      await controller.pause();
    } else {
      if (controller.value.position >= controller.value.duration) {
        await controller.seekTo(Duration.zero);
      }
      await controller.play();
    }
    if (mounted) setState(() {});
  }

  String _timestamp(Duration value) {
    final minutes = value.inMinutes.remainder(60).toString().padLeft(2, '0');
    final seconds = value.inSeconds.remainder(60).toString().padLeft(2, '0');
    return '$minutes:$seconds';
  }

  String _quotePath(String value) => '"${value.replaceAll('"', r'\"')}"';

  Future<void> _exportVideo() async {
    final input = _videoPath;
    if (input == null || _isExporting) return;

    final start = _trimRange.start / 1000;
    final end = _trimRange.end / 1000;
    if (end <= start) {
      setState(() => _status = 'Choose a valid trim range.');
      return;
    }

    setState(() {
      _isExporting = true;
      _status = 'Exporting MP4… keep VeCut open until it finishes.';
      _exportedPath = null;
    });

    try {
      final directory = await getApplicationDocumentsDirectory();
      final output =
          '${directory.path}/${DateTime.now().millisecondsSinceEpoch}_ve_cut_export.mp4';
      final command = '-y -ss $start -to $end -i ${_quotePath(input)} '
          '-c:v mpeg4 -q:v 3 -c:a aac -movflags +faststart ${_quotePath(output)}';
      final session = await FFmpegKit.execute(command);
      final returnCode = await session.getReturnCode();

      if (ReturnCode.isSuccess(returnCode) && await File(output).exists()) {
        if (mounted) {
          setState(() {
            _exportedPath = output;
            _status = 'Export complete! Saved in VeCut app documents.';
          });
        }
      } else {
        final logs = await session.getOutput();
        if (mounted) setState(() => _status = 'Export failed. $logs');
      }
    } catch (error) {
      if (mounted) setState(() => _status = 'Export error: $error');
    } finally {
      if (mounted) setState(() => _isExporting = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final controller = _controller;
    final initialized = controller?.value.isInitialized ?? false;
    final durationMs = _duration.inMilliseconds.toDouble();
    final maxMs = durationMs > 0 ? durationMs : 1.0;
    final currentPosition = controller?.value.position ?? Duration.zero;

    return Scaffold(
      appBar: AppBar(
        title: Text(widget.project.name),
        actions: [
          IconButton(
            tooltip: 'Import video',
            onPressed: _isExporting ? null : _importVideo,
            icon: const Icon(Icons.video_library_outlined),
          ),
          IconButton(
            tooltip: 'Export MP4',
            onPressed: _videoPath == null || _isExporting ? null : _exportVideo,
            icon: const Icon(Icons.file_upload_outlined),
          ),
        ],
      ),
      body: Column(
        children: [
          Expanded(
            flex: 5,
            child: Container(
              width: double.infinity,
              color: Colors.black,
              child: initialized
                  ? Center(
                      child: AspectRatio(
                        aspectRatio: controller!.value.aspectRatio,
                        child: VideoPlayer(controller),
                      ),
                    )
                  : Center(
                      child: Column(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          const Icon(Icons.movie_outlined,
                              size: 64, color: Colors.white38),
                          const SizedBox(height: 12),
                          const Text('Video Preview',
                              style: TextStyle(color: Colors.white70)),
                          const SizedBox(height: 8),
                          Text(
                            '${widget.project.resolution} • ${widget.project.frameRate} FPS',
                            style: const TextStyle(color: Colors.white38),
                          ),
                          const SizedBox(height: 16),
                          FilledButton.icon(
                            onPressed: _importVideo,
                            icon: const Icon(Icons.add),
                            label: const Text('Import Video'),
                          ),
                        ],
                      ),
                    ),
            ),
          ),
          Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
            child: Row(
              children: [
                Text(_timestamp(currentPosition)),
                const Spacer(),
                IconButton(
                  onPressed: initialized ? _togglePlayback : null,
                  icon: Icon(
                    controller?.value.isPlaying == true
                        ? Icons.pause
                        : Icons.play_arrow,
                    size: 32,
                  ),
                ),
                const Spacer(),
                Text(_timestamp(_duration)),
              ],
            ),
          ),
          if (initialized)
            VideoProgressIndicator(
              controller!,
              allowScrubbing: true,
              padding: const EdgeInsets.symmetric(horizontal: 16),
              colors: const VideoProgressColors(
                playedColor: Colors.deepPurpleAccent,
                bufferedColor: Colors.white30,
                backgroundColor: Colors.white12,
              ),
            ),
          Expanded(
            flex: 3,
            child: Container(
              width: double.infinity,
              color: const Color(0xFF181820),
              padding: const EdgeInsets.all(16),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Row(
                    children: [
                      const Text('TIMELINE',
                          style: TextStyle(
                              fontWeight: FontWeight.bold,
                              letterSpacing: 1.5)),
                      const Spacer(),
                      Text('${_tracks.length}/$_freeTrackLimit tracks'),
                      const SizedBox(width: 8),
                      PopupMenuButton<String>(
                        tooltip: 'Add stack',
                        icon: const Icon(Icons.add_box_outlined),
                        onSelected: (value) {
                          switch (value) {
                            case 'Overlay':
                              _addTrack(value, Icons.layers_outlined);
                            case 'Text':
                              _addTrack(value, Icons.title_rounded);
                            case 'Audio':
                              _addTrack(value, Icons.music_note_rounded);
                            case 'Sticker':
                              _addTrack(value, Icons.sticky_note_2_outlined);
                            case 'Effect':
                              _addTrack(value, Icons.auto_awesome_outlined);
                          }
                        },
                        itemBuilder: (context) => const [
                          PopupMenuItem(value: 'Overlay', child: Text('Overlay stack')),
                          PopupMenuItem(value: 'Text', child: Text('Text stack')),
                          PopupMenuItem(value: 'Audio', child: Text('Audio stack')),
                          PopupMenuItem(value: 'Sticker', child: Text('Sticker stack')),
                          PopupMenuItem(value: 'Effect', child: Text('Effect stack')),
                        ],
                      ),
                    ],
                  ),
                  const SizedBox(height: 8),
                  SingleChildScrollView(
                    scrollDirection: Axis.horizontal,
                    child: Row(
                      children: [
                        _PlaceholderToolIcon(
                          icon: Icons.undo_rounded,
                          label: 'Undo',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.redo_rounded,
                          label: 'Redo',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.content_cut_rounded,
                          label: 'Split',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.music_note_rounded,
                          label: 'Audio',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.title_rounded,
                          label: 'Text',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.auto_awesome_rounded,
                          label: 'Effects',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.sticky_note_2_outlined,
                          label: 'Stickers',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.layers_rounded,
                          label: 'Layers',
                        ),
                        _PlaceholderToolIcon(
                          icon: Icons.settings_outlined,
                          label: 'Settings',
                        ),
                      ],
                    ),
                  ),
                  const SizedBox(height: 8),
                  SizedBox(
                    height: 132,
                    child: ListView.separated(
                      itemCount: _tracks.length,
                      separatorBuilder: (context, index) =>
                          const SizedBox(height: 6),
                      itemBuilder: (context, index) {
                        final track = _tracks[index];
                        final isVideo = track.label == 'Video';
                        return Container(
                          height: 42,
                          decoration: BoxDecoration(
                            color: const Color(0xFF292432),
                            borderRadius: BorderRadius.circular(7),
                            border: Border.all(
                              color: isVideo
                                  ? Colors.deepPurpleAccent
                                  : Colors.white12,
                            ),
                          ),
                          child: Row(
                            children: [
                              const SizedBox(width: 10),
                              Icon(track.icon, size: 17, color: Colors.white70),
                              const SizedBox(width: 8),
                              SizedBox(
                                width: 66,
                                child: Text(
                                  track.label,
                                  maxLines: 1,
                                  overflow: TextOverflow.ellipsis,
                                  style: const TextStyle(
                                    color: Colors.white70,
                                    fontSize: 12,
                                  ),
                                ),
                              ),
                              const VerticalDivider(
                                color: Colors.white12,
                                width: 12,
                                indent: 7,
                                endIndent: 7,
                              ),
                              Expanded(
                                child: Align(
                                  alignment: Alignment.centerLeft,
                                  child: Container(
                                    height: 24,
                                    margin: const EdgeInsets.symmetric(horizontal: 6),
                                    padding: const EdgeInsets.symmetric(horizontal: 8),
                                    decoration: BoxDecoration(
                                      color: isVideo
                                          ? Colors.deepPurple.withValues(alpha: 0.35)
                                          : Colors.white.withValues(alpha: 0.06),
                                      borderRadius: BorderRadius.circular(5),
                                    ),
                                    alignment: Alignment.centerLeft,
                                    child: Text(
                                      isVideo
                                          ? (_videoPath == null
                                              ? 'Video clip'
                                              : _videoPath!.split(Platform.pathSeparator).last)
                                          : 'Empty ${track.label.toLowerCase()} stack',
                                      maxLines: 1,
                                      overflow: TextOverflow.ellipsis,
                                      style: const TextStyle(
                                        color: Colors.white60,
                                        fontSize: 10,
                                      ),
                                    ),
                                  ),
                                ),
                              ),
                              if (!isVideo)
                                IconButton(
                                  tooltip: 'Remove stack',
                                  visualDensity: VisualDensity.compact,
                                  onPressed: () => setState(() => _tracks.removeAt(index)),
                                  icon: const Icon(Icons.close, size: 16),
                                )
                              else
                                const SizedBox(width: 42),
                            ],
                          ),
                        );
                      },
                    ),
                  ),
                  const SizedBox(height: 14),
                  const Text('TRIM CLIP',
                      style: TextStyle(
                          fontWeight: FontWeight.bold, letterSpacing: 1.2)),
                  RangeSlider(
                    values: _trimRange,
                    min: 0,
                    max: maxMs,
                    divisions: maxMs > 1 ? maxMs.round().clamp(1, 1000).toInt() : 1,
                    labels: RangeLabels(
                      _timestamp(Duration(milliseconds: _trimRange.start.round())),
                      _timestamp(Duration(milliseconds: _trimRange.end.round())),
                    ),
                    onChanged: initialized
                        ? (value) => setState(() => _trimRange = value)
                        : null,
                  ),
                  const Spacer(),
                  Text(
                    _status,
                    maxLines: 2,
                    overflow: TextOverflow.ellipsis,
                    style: const TextStyle(color: Colors.white70, fontSize: 12),
                  ),
                  if (_isExporting) const LinearProgressIndicator(),
                  if (_exportedPath != null)
                    Text(
                      _exportedPath!,
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                      style: const TextStyle(color: Colors.greenAccent, fontSize: 11),
                    ),
                  const SizedBox(height: 6),
                  SizedBox(
                    width: double.infinity,
                    child: FilledButton.icon(
                      onPressed: _videoPath == null || _isExporting
                          ? null
                          : _exportVideo,
                      icon: const Icon(Icons.movie_creation_outlined),
                      label: Text(_isExporting ? 'Exporting…' : 'Export MP4'),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _TimelineTrack {
  const _TimelineTrack({required this.label, required this.icon});

  final String label;
  final IconData icon;
}

class _PlaceholderToolIcon extends StatelessWidget {
  const _PlaceholderToolIcon({
    required this.icon,
    required this.label,
  });

  final IconData icon;
  final String label;

  @override
  Widget build(BuildContext context) {
    return Padding(
      padding: const EdgeInsets.only(right: 8),
      child: Tooltip(
        message: label,
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            IconButton(
              onPressed: null,
              icon: Icon(icon),
              visualDensity: VisualDensity.compact,
            ),
            Text(
              label,
              style: const TextStyle(fontSize: 10, color: Colors.white54),
            ),
          ],
        ),
      ),
    );
  }
}
