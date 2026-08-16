# topology_query - Multi-PPU System Topology Report

## Description

This sample generates a structured multi-PPU system topology report containing four sections:

1. **Device Overview Table** — name, compute unit count, memory capacity, and clock frequency for each PPU
2. **P2P Connectivity Matrix** — ASCII matrix visualization showing inter-device access capability and atomic support
3. **Performance Grouping** — grouped by P2P performance tier to help select optimal device pairs
4. **Host <-> Device Properties** — host atomic, UVA, and managed memory support status

Used to understand system interconnect topology before multi-device workload distribution, providing decision basis for data placement and migration strategies.

## Key Concepts

Peer-to-Peer Interconnect, Topology Discovery, Device Property Query, Multi-PPU System.

## Example Output

```
[topology_query] Multi-PPU System Topology Report
=================================================

  Detected 4 PPU device(s)

+-----+------------------------+------+----------+-----------+
| PPU | Name                   |  CUs | Mem (MB) | Clock MHz |
+-----+------------------------+------+----------+-----------+
|   0 | PPU-X100               |   80 |    32768 |      1500 |
|   1 | PPU-X100               |   80 |    32768 |      1500 |
  ...

  P2P Access Matrix (row->col):  '.' no access  'A' access  '*' access+atomic

       P0  P1  P2  P3
  P0    -   *   *   A
  P1    *   -   A   *
  ...
```.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API

hggcGetDeviceCount, hggcGetDeviceProperties, hggcDeviceGetAttribute, hggcDeviceGetP2PAttribute

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
