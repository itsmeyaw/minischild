import 'package:flutter/material.dart';

import 'ble_protocol.dart';
import 'device_link.dart';
import 'settings_editor.dart';

class WifiScreen extends StatefulWidget {
  const WifiScreen({super.key, required this.link, required this.settings});

  final DeviceLink link;
  final Settings settings;

  @override
  State<WifiScreen> createState() => _WifiScreenState();
}

class _WifiScreenState extends State<WifiScreen> {
  final _formKey = GlobalKey<FormState>();
  bool _saving = false;
  String _ssid = '';
  String _password = '';

  Future<void> _save() async {
    if (_saving || !_formKey.currentState!.validate()) return;
    _formKey.currentState!.save();
    setState(() => _saving = true);
    await writeSettingsAndClose(
      context,
      widget.link,
      widget.settings.copyWith(wifiSsid: _ssid),
      wifiPassword: _password,
      // A password equal to the stored one can't be seen here, so this may
      // reconnect to a Device that didn't restart. It still works.
      restarts: _ssid != widget.settings.wifiSsid || _password.isNotEmpty,
    );
    if (mounted) setState(() => _saving = false);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('WiFi network'),
        actions: [
          SaveAction(saving: _saving, onPressed: _saving ? null : _save),
        ],
      ),
      body: Form(
        key: _formKey,
        child: ListView(
          padding: const EdgeInsets.all(16),
          children: [
            TextFormField(
              initialValue: widget.settings.wifiSsid,
              autocorrect: false,
              textInputAction: TextInputAction.next,
              decoration: const InputDecoration(
                labelText: 'Network name',
                border: OutlineInputBorder(),
              ),
              onSaved: (value) => _ssid = value ?? '',
              validator: (value) {
                if (value == null || value.isEmpty) {
                  return 'Enter the network name.';
                }
                if (!isWifiSsid(value)) {
                  return 'Network names are at most 32 bytes.';
                }
                return null;
              },
            ),
            const SizedBox(height: 16),
            TextFormField(
              obscureText: true,
              enableSuggestions: false,
              autocorrect: false,
              textInputAction: TextInputAction.done,
              decoration: const InputDecoration(
                labelText: 'Password',
                helperText: 'Leave empty to keep the current password.',
                border: OutlineInputBorder(),
              ),
              onSaved: (value) => _password = value ?? '',
              onFieldSubmitted: (_) => _save(),
              validator: (value) {
                if (value != null && !isWifiPassword(value)) {
                  return 'Passwords are at most 64 bytes.';
                }
                return null;
              },
            ),
          ],
        ),
      ),
    );
  }
}
