# hg_jpeg - Batch JPEG Decoder with Pixel Statistics

## Description

This sample demonstrates hgjpeg batch JPEG decoding with on-device pixel statistics computation. JPEG files are loaded from a directory, decoded in batches via `hgjpegDecodeBatched`, and per-channel pixel statistics (min/max/mean) are computed using a custom PPU reduction kernel — demonstrating hgjpeg + kernel interoperability.

Pipeline:
1. Load JPEG files from a directory
2. Query image metadata (dimensions, channels, subsampling) via `hgjpegGetImageInfo`
3. Batch-decode via `hgjpegDecodeBatched`
4. Compute per-channel pixel statistics (min/max/mean) on device
5. Print summary table with metadata + statistics

## Key Concepts

HGGCJPEG Library, Batch Decoding, Pixel Statistics, Device Kernel Reduction, Image Processing

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcMemset, hggcFree, hggcDeviceSynchronize

### HGGCJPEG API
hgjpegCreateEx, hgjpegJpegStateCreate, hgjpegDecodeBatchedInitialize, hgjpegGetImageInfo, hgjpegDecodeBatched, hgjpegJpegStateDestroy, hgjpegDestroy

## Dependencies for Build/Run
HGGCJPEG

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
