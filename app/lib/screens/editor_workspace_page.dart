import 'package:flutter/material.dart';
import '../models/ve_cut_project.dart';

class EditorWorkspacePage extends StatelessWidget {
  const EditorWorkspacePage({super.key, required this.project});

  final VeCutProject project;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(project.name),
        actions: [
          IconButton(onPressed: () {}, icon: const Icon(Icons.undo)),
          IconButton(onPressed: () {}, icon: const Icon(Icons.redo)),
          IconButton(
            onPressed: () {},
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
              child: Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    const Icon(
                      Icons.movie_outlined,
                      size: 64,
                      color: Colors.white38,
                    ),
                    const SizedBox(height: 12),
                    const Text(
                      'Video Preview',
                      style: TextStyle(color: Colors.white70),
                    ),
                    const SizedBox(height: 8),
                    Text(
                      '${project.resolution} • ${project.frameRate} FPS',
                      style: const TextStyle(color: Colors.white38),
                    ),
                  ],
                ),
              ),
            ),
          ),
          const Divider(height: 1),
          Padding(
            padding: const EdgeInsets.all(16),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text('00:00 / 00:00'),
                IconButton(
                  onPressed: () {},
                  icon: const Icon(Icons.play_arrow, size: 32),
                ),
                const Text('16:9'),
              ],
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
                  const Text(
                    'TIMELINE',
                    style: TextStyle(
                      fontWeight: FontWeight.bold,
                      letterSpacing: 1.5,
                    ),
                  ),
                  const SizedBox(height: 20),
                  Container(
                    height: 65,
                    width: double.infinity,
                    decoration: BoxDecoration(
                      color: const Color(0xFF292432),
                      borderRadius: BorderRadius.circular(8),
                      border: Border.all(color: Colors.deepPurpleAccent),
                    ),
                    child: const Center(
                      child: Text(
                        'Video Track 1',
                        style: TextStyle(color: Colors.white70),
                      ),
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
