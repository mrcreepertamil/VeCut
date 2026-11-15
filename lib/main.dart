import 'package:flutter/material.dart';
import 'package:flutter_svg/flutter_svg.dart';

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
      builder: (context, child) => Stack(
        fit: StackFit.expand,
        children: [
          SvgPicture.asset(
            'assets/textures/ve_cut_graphite.svg',
            fit: BoxFit.cover,
          ),
          const ColoredBox(color: Color(0x990B0A10)),
          child ?? const SizedBox.shrink(),
        ],
      ),
      home: const EditorHomePage(),
    );
  }
}
