import 'dart:async';

import 'package:flutter/material.dart';

import 'ble_protocol.dart';
import 'device_link.dart';
import 'settings_editor.dart';
import 'status_message.dart';
import 'transit_provider.dart';

class StationSearchScreen extends StatefulWidget {
  const StationSearchScreen({
    super.key,
    required this.provider,
    required this.link,
    required this.settings,
  });

  final TransitProvider provider;
  final DeviceLink link;
  final Settings settings;

  @override
  State<StationSearchScreen> createState() => _StationSearchScreenState();
}

sealed class _Results {
  const _Results();
}

class _NotSearched extends _Results {
  const _NotSearched();
}

class _Found extends _Results {
  const _Found(this.stations);

  final List<Station> stations;
}

class _NoMatches extends _Results {
  const _NoMatches();
}

class _Unreachable extends _Results {
  const _Unreachable();
}

class _StationSearchScreenState extends State<StationSearchScreen> {
  _Results _results = const _NotSearched();
  Station? _chosen;
  bool _searching = false;
  bool _saving = false;
  String _query = '';
  int _searchId = 0;
  Timer? _debounce;

  @override
  void dispose() {
    _debounce?.cancel();
    super.dispose();
  }

  void _onChanged(String text) {
    _debounce?.cancel();
    _debounce = Timer(const Duration(milliseconds: 400), () => _search(text));
  }

  void _onSubmitted(String text) {
    _debounce?.cancel();
    _search(text);
  }

  Future<void> _search(String text) async {
    final query = text.trim();
    final searchId = ++_searchId;
    _query = query;
    if (query.isEmpty) {
      _showResults(const _NotSearched());
      return;
    }
    setState(() => _searching = true);
    final results = await _fetch(query);
    if (mounted && searchId == _searchId) _showResults(results);
  }

  Future<_Results> _fetch(String query) async {
    try {
      final stations = await widget.provider
          .searchStations(query)
          .timeout(const Duration(seconds: 10));
      return stations.isEmpty ? const _NoMatches() : _Found(stations);
    } catch (error) {
      debugPrint('Searching Stations for “$query” failed: $error');
      return const _Unreachable();
    }
  }

  void _showResults(_Results results) {
    setState(() {
      _searching = false;
      _results = results;
      if (results is! _Found ||
          !results.stations.any((station) => station.id == _chosen?.id)) {
        _chosen = null;
      }
    });
  }

  void _choose(Station station) {
    FocusScope.of(context).unfocus();
    setState(() => _chosen = station);
  }

  Future<void> _save(Station station) async {
    setState(() => _saving = true);
    await writeSettingsAndClose(
      context,
      widget.link,
      widget.settings.copyWith(stationId: station.id),
    );
    if (mounted) setState(() => _saving = false);
  }

  @override
  Widget build(BuildContext context) {
    final chosen = _chosen;
    return Scaffold(
      appBar: AppBar(
        title: const Text('Station'),
        actions: [
          SaveAction(
            saving: _saving,
            onPressed: chosen == null || _saving ? null : () => _save(chosen),
          ),
        ],
      ),
      body: Column(
        children: [
          Padding(
            padding: const EdgeInsets.all(16),
            child: TextField(
              autofocus: true,
              textInputAction: TextInputAction.search,
              onChanged: _onChanged,
              onSubmitted: _onSubmitted,
              decoration: const InputDecoration(
                labelText: 'Search Stations',
                prefixIcon: Icon(Icons.search),
                border: OutlineInputBorder(),
              ),
            ),
          ),
          if (_searching) const LinearProgressIndicator(),
          Expanded(child: _body()),
        ],
      ),
    );
  }

  Widget _body() {
    final theme = Theme.of(context);
    return switch (_results) {
      _NotSearched() => Center(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Text(
            'Type part of a Station’s name, like “Marienpl”.',
            style: theme.textTheme.bodyMedium?.copyWith(
              color: theme.colorScheme.onSurfaceVariant,
            ),
            textAlign: TextAlign.center,
          ),
        ),
      ),
      _Found(:final stations) => ListView(
        children: [
          for (final station in stations)
            ListTile(
              title: Text(station.name),
              selected: station.id == _chosen?.id,
              trailing: station.id == _chosen?.id
                  ? const Icon(Icons.check)
                  : null,
              onTap: () => _choose(station),
            ),
        ],
      ),
      _NoMatches() => StatusMessage(
        icon: const Icon(Icons.search_off, size: 48),
        headline: 'No Stations found',
        message:
            'No Station matches “$_query”. Try a shorter or different name.',
      ),
      _Unreachable() => StatusMessage(
        icon: const Icon(Icons.cloud_off, size: 48),
        headline: 'Couldn’t reach the provider',
        message: 'Check your phone’s internet connection, then try again.',
        action: FilledButton(
          onPressed: () => _search(_query),
          child: const Text('Try again'),
        ),
      ),
    };
  }
}
