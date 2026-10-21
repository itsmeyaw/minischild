import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:minischild/main.dart';

void main() {
  testWidgets('shows the Minischild placeholder screen', (tester) async {
    await tester.pumpWidget(const MinischildApp());

    expect(find.widgetWithText(AppBar, 'Minischild'), findsOneWidget);
    expect(find.text('Device setup is coming soon'), findsOneWidget);
  });
}
