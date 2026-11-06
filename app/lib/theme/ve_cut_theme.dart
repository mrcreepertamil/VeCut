
import 'package:flutter/material.dart';

class VeCutTheme {
  static const Color background = Color(0xFF101014);
  static const Color surface = Color(0xFF181820);
  static const Color accent = Color(0xFF8B5CF6);

  static ThemeData darkTheme = ThemeData(
    brightness: Brightness.dark,
    scaffoldBackgroundColor: background,
    colorScheme: ColorScheme.fromSeed(
      seedColor: accent,
      brightness: Brightness.dark,
    ),
    appBarTheme: const AppBarTheme(
      backgroundColor: surface,
      foregroundColor: Colors.white,
    ),
    useMaterial3: true,
  );
}
