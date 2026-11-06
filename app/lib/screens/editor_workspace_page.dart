
import 'package:flutter/material.dart';

class EditorWorkspacePage extends StatelessWidget {
  const EditorWorkspacePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Untitled Project'),
        actions: [
          IconButton(
            onPressed: () {},
            icon: const Icon(Icons.undo),
          ),
          IconButton(
            onPressed: () {},
            icon: const Icon(Icons.redo),
          ),
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
              child: const Center(
                child: Column(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Icon(
                      Icons.movie_outlined,
                      size: 64,
                      color: Colors.white38,
                    ),
                    SizedBox(height: 12),
                    Text(
                      'Video Preview',
                      style: TextStyle(color: Colors.white70),
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
                      border: Border.all(
                        color: Colors.deepPurpleAccent,
                      ),
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
