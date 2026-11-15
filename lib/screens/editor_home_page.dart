import 'package:flutter/material.dart';
import 'project_setup_page.dart';

class EditorHomePage extends StatelessWidget {
  const EditorHomePage({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text(
          'VeCut',
          style: TextStyle(fontWeight: FontWeight.bold, letterSpacing: 0.4),
        ),
      ),
      body: Center(
        child: Container(
          margin: const EdgeInsets.all(20),
          padding: const EdgeInsets.symmetric(horizontal: 24, vertical: 30),
          decoration: BoxDecoration(
            color: const Color(0xB8181520),
            borderRadius: BorderRadius.circular(24),
            border: Border.all(color: const Color(0x557C3AED)),
            boxShadow: const [
              BoxShadow(
                color: Color(0x332D1260),
                blurRadius: 28,
                spreadRadius: 2,
              ),
            ],
          ),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: [
              Container(
                width: 104,
                height: 104,
                decoration: BoxDecoration(
                  gradient: const LinearGradient(
                    colors: [Color(0xFFB66BFF), Color(0xFF6D28D9)],
                    begin: Alignment.topLeft,
                    end: Alignment.bottomRight,
                  ),
                  borderRadius: BorderRadius.circular(28),
                  boxShadow: const [
                    BoxShadow(color: Color(0x667C3AED), blurRadius: 24),
                  ],
                ),
                child: const Icon(
                  Icons.movie_creation_outlined,
                  size: 58,
                  color: Colors.white,
                ),
              ),
              const SizedBox(height: 22),
              const Text(
                'Welcome to VeCut',
                style: TextStyle(fontSize: 26, fontWeight: FontWeight.bold),
              ),
              const SizedBox(height: 10),
              const Text(
                'Your creative video editing workspace.',
                textAlign: TextAlign.center,
                style: TextStyle(color: Colors.white70),
              ),
              const SizedBox(height: 30),
              SizedBox(
                width: double.infinity,
                child: FilledButton.icon(
                  style: FilledButton.styleFrom(
                    backgroundColor: const Color(0xFF812BEB),
                    foregroundColor: Colors.white,
                    padding: const EdgeInsets.symmetric(vertical: 15),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(14),
                    ),
                  ),
                  onPressed: () {
                    Navigator.push(
                      context,
                      MaterialPageRoute(
                        builder: (context) => const ProjectSetupPage(),
                      ),
                    );
                  },
                  icon: const Icon(Icons.add),
                  label: const Text('New Project'),
                ),
              ),
              const SizedBox(height: 12),
              SizedBox(
                width: double.infinity,
                child: OutlinedButton.icon(
                  style: OutlinedButton.styleFrom(
                    foregroundColor: Colors.white,
                    side: const BorderSide(color: Color(0x887C3AED)),
                    padding: const EdgeInsets.symmetric(vertical: 15),
                    shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(14),
                    ),
                  ),
                  onPressed: () {},
                  icon: const Icon(Icons.folder_open),
                  label: const Text('Open Project'),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
