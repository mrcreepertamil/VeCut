
import 'package:flutter/material.dart';
import 'screens/editor_home_page.dart';
import 'theme/ve_cut_theme.dart';

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
      theme: VeCutTheme.darkTheme,
      home: const EditorHomePage(),
    );
  }
}
