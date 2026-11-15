import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:vecut/main.dart';

void main() {
  testWidgets('creates a project and opens the editor workspace',
      (WidgetTester tester) async {
    await tester.pumpWidget(const VeCutApp());

    expect(find.text('Welcome to VeCut'), findsOneWidget);

    await tester.tap(find.text('New Project'));
    await tester.pumpAndSettle();

    await tester.enterText(find.byType(TextField), 'My First Edit');
    await tester.tap(find.text('Create Project'));
    await tester.pumpAndSettle();

    expect(find.text('My First Edit'), findsOneWidget);
    expect(find.text('1080p • 30 FPS'), findsOneWidget);
  });
}
