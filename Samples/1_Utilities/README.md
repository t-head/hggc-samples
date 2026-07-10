# 1. Utilities

This directory contains utility samples for querying device properties and system topology. Some directories provide multiple build variants (Runtime API, Driver API, etc.) within the same sample.

## Device Query

### device_query
Enumerates properties of HGGC devices present in the system, providing 2 build modes (within the same directory):
- `device_query` — Runtime API implementation
- `device_query_drv` — Driver API implementation

## System Topology

### topology_query
Generates a structured multi-PPU system topology report: device overview table, P2P connectivity matrix (ASCII visualization), performance tier grouping, and Host <-> Device property summary.
