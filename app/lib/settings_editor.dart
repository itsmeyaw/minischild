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
  try {
    await link.writeSettings(settings, wifiPassword: wifiPassword);
  } on DeviceLinkException {
    if (context.mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text(
            'Couldn’t save. Make sure the Device is on and nearby, then try again.',
          ),
        ),
      );
    }
    return;
  }
  if (context.mounted) Navigator.of(context).pop(true);
}
