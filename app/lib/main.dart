import 'package:flutter/material.dart';

void main() => runApp(const MinischildApp());

class MinischildApp extends StatelessWidget {
  const MinischildApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Minischild',
      home: Scaffold(
        appBar: AppBar(title: const Text('Minischild')),
        body: const Center(child: Text('Device setup is coming soon')),
      ),
    );
  }
}
