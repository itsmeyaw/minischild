import 'package:flutter/material.dart';

class StatusMessage extends StatelessWidget {
  const StatusMessage({
    super.key,
    required this.icon,
    required this.headline,
    required this.message,
    this.action,
  });

  final Widget icon;
  final String headline;
  final String message;
  final Widget? action;

  @override
  Widget build(BuildContext context) {
    final textTheme = Theme.of(context).textTheme;
    return Center(
      child: Padding(
        padding: const EdgeInsets.all(24),
        child: Column(
          mainAxisSize: MainAxisSize.min,
          spacing: 16,
          children: [
            icon,
            Text(
              headline,
              style: textTheme.titleMedium,
              textAlign: TextAlign.center,
            ),
            Text(
              message,
              style: textTheme.bodyMedium,
              textAlign: TextAlign.center,
            ),
            ?action,
          ],
        ),
      ),
    );
  }
}
