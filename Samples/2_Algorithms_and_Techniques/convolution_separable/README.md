# convolution_separable - Separable Gaussian Blur

## Description

This sample demonstrates **Separable Convolution** applied to image processing, using Gaussian blur as an example:

- Decomposes 2-D convolution into horizontal + vertical two-pass 1-D convolution (complexity reduced from O(N²K²) to O(N²K))
- Row convolution kernel: each block loads TILE_W pixels + left/right halo into shared memory
- Column convolution kernel: transposed layout ensures coalesced reads in the vertical direction
- 1-D kernel weights stored in `__constant__` memory for broadcast
- Gaussian kernel generated from sigma parameter on the host side, not random values

## Key Concepts

Separable Convolution, Shared Memory Halo Loading, Constant Memory Broadcast, Image Processing, Gaussian Blur.

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize, hggcMemcpyToSymbol

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
