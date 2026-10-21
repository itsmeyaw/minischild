import 'package:flutter/material.dart';

import 'ble_protocol.dart';
import 'device_link.dart';

class SaveAction extends StatelessWidget {
  const SaveAction({super.key, required this.saving, required this.onPressed});

  final bool saving;
  final VoidCallback? onPressed;

  @override
  Widget build(BuildContext context) {
    if (saving) {
      return Padding(
        padding: const EdgeInsets.symmetric(horizontal: 12),
        child: Semantics(
          label: 'Saving',
          child: const SizedBox(
            width: 20,
            height: 20,
            child: CircularProgressIndicator(strokeWidth: 2),
          ),
        ),
      );
    }
    return TextButton(onPressed: onPressed, child: const Text('Save'));
  }
}

Future<void> writeSettingsAndClose(
  BuildContext context,
  DeviceLink link,
  Settings settings, {
  String wifiPassword = '',
}) async {
  final String failure;
  try {
    await link.writeSettings(settings, wifiPassword: wifiPassword);
    if (context.mounted) Navigator.of(context).pop(true);
    return;
  } on FormatException {
    failure =
        'This is too much for the Device to store. Choose fewer Route Directions or Platforms, or use a shorter password.';
  } on DeviceLinkException {
    failure =
        'Couldn’t save. Make sure the Device is on and nearby, then try again.';
  }
  if (context.mounted) {
    ScaffoldMessenger.of(
      context,
    ).showSnackBar(SnackBar(content: Text(failure)));
  }
}
