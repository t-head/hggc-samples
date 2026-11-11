# array_roundtrip - 1D/2D/3D Array Round Trips and Offset Verification

## Description

Uses single-plane int32 `hggcArrayDefault` arrays on the selected device to verify allocation, channel/shape/flag queries, full round trips, subregion updates/reads, and untouched regions in all three dimensions. Deterministic integer inputs are compared exactly, element by element. The 2D/3D tests also check sentinels in host row and slice padding.

| Dimension | Array Shape (Elements) | Host Layout | Subregion |
|---|---|---|---|
| 1D | 37 | Contiguous | X=3, width 29 |
| 2D | 37×11 | Row pitch 41 elements | (3,2), 29×7 |
| 3D | 19×9×5 | Row pitch 23 elements, slice height 11 rows | (1,2,1), 13×5×3 |

The pointer source for the 3D subregion also has a nonzero offset `(2*sizeof(int32),1,1)`, checking the different units used on the array and pointer sides.

## Unit Rules

- Array allocation width and extent.width are measured in elements defined by the channel descriptor.
- The X offsets and count of `hggcMemcpyToArray` / `hggcMemcpyFromArray` are in bytes. These legacy APIs are marked deprecated in the toolkit; this sample retains them for 1D instructional coverage.
- For 2D copies, pitch, X offsets, and width are in bytes; Y offsets and height are in rows.
- For `hggcMemcpy3D` involving an array, extent.width is in array elements. pos.x is always in bytes, not elements, and must be a multiple of the element size. Pitch is in bytes; Y/Z are in rows/slices.
- `hggcExtent`, `hggcPitchedPtr`, and `hggcPos` use aggregate initialization of toolkit structures, not undeclared helper constructors.

## Key Concepts

Array, Channel Descriptor, Extent, Pitch, Offsets, Round-Trip Verification, Padding Sentinels.

## Supported Platforms and Dependencies

Linux, x86_64; source targets ppu001 and ppu0015. Texture, surface, layered, and sparse arrays are outside this sample's scope.

## HGGC Runtime API

hggcSetDevice, hggcMallocArray, hggcMalloc3DArray, hggcArrayGetInfo, hggcGetChannelDesc, hggcCreateChannelDesc, hggcMemcpyToArray, hggcMemcpyFromArray, hggcMemcpy2DToArray, hggcMemcpy2DFromArray, hggcMemcpy3D, hggcFreeArray

## Prerequisites

Please download and install the T-Head SAIL toolkit for your platform. Requires SAIL 2.2.0 or later.

## Building and Running

Configure CMake from the repository root, then build the `array_roundtrip` target. Run it with:

```bash
./array_roundtrip -device=0
```

Any API error or data mismatch returns a nonzero exit code.
