# Minischild

A small always-on display that shows the upcoming public transport departures at one MVV station. Terms follow GTFS / GTFS Realtime wherever a matching concept exists.

## Language

**Station**:
The place whose departures are shown, identified by its global ID (e.g. `de:09162:2`); a GTFS stop with `location_type=1`.
_Avoid_: Stop, halt

**Route**:
A service as riders know it, identified to riders by its short name, e.g. `S1`, `U3`, `19`.
_Avoid_: Line, service

**Route Direction**:
A Route travelling in one of its two directions; the unit by which departures are selected.
_Avoid_: Line, stateless, route (when the direction matters)

**Headsign**:
The text shown to riders naming where a Departure is heading.
_Avoid_: Destination, direction

**Platform**:
Where at the Station a Departure leaves from.
_Avoid_: Track, Gleis

**Departure**:
One vehicle of a Route Direction leaving the Station, with a scheduled time and, when realtime data exists, a predicted time.
_Avoid_: Trip, connection

**Delay**:
The predicted time of a Departure minus its scheduled time; positive means late.

**No Data**:
The state of a Departure for which no realtime prediction exists, so only its scheduled time is known.
_Avoid_: Unknown, offline

**Departure Board**:
The set of upcoming Departures at the Station at one moment, and the screen that shows them.
_Avoid_: Schedule, timetable, widget

**Board Selection**:
The Station, Route Directions and Platforms chosen to be shown on a Device; an empty list of Route Directions or Platforms means all of them.
_Avoid_: Config, settings, filter

**Device**:
The always-on display hardware that shows one Departure Board.
_Avoid_: Board, unit, ESP

**Companion App**:
The phone app that sets a Device's Board Selection.
_Avoid_: Client, configurator
