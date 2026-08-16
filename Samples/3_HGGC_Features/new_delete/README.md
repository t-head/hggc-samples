# new_delete - Device-Side Linked List (new/delete + Virtual Functions)

## Description

This sample demonstrates **C++ dynamic memory management and polymorphism** on the PPU device side:

Constructs a **singly linked list** on the device, with nodes allocated on the device heap via `new`, polymorphic behavior achieved through **virtual functions** during traversal, and freed via `delete`.

- **DataNode**: `contribute()` returns `value`
- **DoubleNode**: `contribute()` returns `value * 2` (polymorphic derivation)

C++ features demonstrated:
- Device-side `new` / `delete` (device heap allocation)
- Virtual functions (virtual dispatch on device)
- Virtual destructors (correct deallocation of derived classes)
- Pointer traversal (linked list traversal)
- `hggcDeviceSetLimit(hggcLimitMallocHeapSize)` to configure device heap size

## Key Concepts

Device Heap (Dynamic Memory), C++ Polymorphism (Virtual Functions), Linked List.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcDeviceSetLimit, hggcMalloc, hggcMemset, hggcMemcpy, hggcFree, hggcDeviceSynchronize

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
