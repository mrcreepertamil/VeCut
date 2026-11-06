
import 'package:flutter/material.dart';
import 'screens/editor_home_page.dart';

void main() {
  runApp(const VeCutApp());
}

class VeCutApp extends StatelessWidget {
  const VeCutApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'VeCut',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        brightness: Brightness.dark,
        colorSchemeSeed: Colors.deepPurple,
        useMaterial3: true,
      ),
      home: const EditorHomePage(),
    );
  }
}
