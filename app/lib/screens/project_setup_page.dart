
import 'package:flutter/material.dart';

class ProjectSetupPage extends StatefulWidget {
  const ProjectSetupPage({super.key});

  @override
  State<ProjectSetupPage> createState() =>
      _ProjectSetupPageState();
}

class _ProjectSetupPageState extends State<ProjectSetupPage> {
  final TextEditingController nameController =
      TextEditingController();

  String resolution = '1080p';
  int frameRate = 30;

  @override
  void dispose() {
    nameController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('New Project'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(20),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            const Text(
              'Project Name',
              style: TextStyle(fontSize: 16),
            ),
            const SizedBox(height: 8),
            TextField(
              controller: nameController,
              decoration: const InputDecoration(
                hintText: 'Enter project name',
                border: OutlineInputBorder(),
              ),
            ),
            const SizedBox(height: 25),
            const Text(
              'Resolution',
              style: TextStyle(fontSize: 16),
            ),
            const SizedBox(height: 8),
            DropdownButtonFormField<String>(
              value: resolution,
              decoration: const InputDecoration(
                border: OutlineInputBorder(),
              ),
              items: ['720p', '1080p', '1440p', '4K']
                  .map((value) => DropdownMenuItem(
                        value: value,
                        child: Text(value),
                      ))
                  .toList(),
              onChanged: (value) {
                if (value != null) {
                  setState(() => resolution = value);
                }
              },
            ),
            const SizedBox(height: 25),
            const Text(
              'Frame Rate',
              style: TextStyle(fontSize: 16),
            ),
            const SizedBox(height: 8),
            DropdownButtonFormField<int>(
              value: frameRate,
              decoration: const InputDecoration(
                border: OutlineInputBorder(),
              ),
              items: [24, 25, 30, 50, 60]
                  .map((value) => DropdownMenuItem(
                        value: value,
                        child: Text('$value FPS'),
                      ))
                  .toList(),
              onChanged: (value) {
                if (value != null) {
                  setState(() => frameRate = value);
                }
              },
            ),
            const Spacer(),
            SizedBox(
              width: double.infinity,
              child: FilledButton.icon(
                onPressed: () {},
                icon: const Icon(Icons.add),
                label: const Text('Create Project'),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
