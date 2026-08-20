# simple_acfft - Spectrum Analysis & Frequency Domain Filtering

## Description

This sample demonstrates ACFFT for spectrum analysis and frequency domain filtering. A composite signal (sum of sine waves + noise) is transformed to the frequency domain, analyzed to detect dominant frequencies, filtered with a brick-wall low-pass filter, and reconstructed via inverse FFT.

Pipeline:
1. Build composite signal on host (3 sine waves at known frequencies + white noise)
2. Forward FFT via `acfftExecC2C`
3. Device kernel: compute magnitude spectrum
4. Detect peak frequency bins (host-side search)
5. Device kernel: apply brick-wall low-pass filter
6. Inverse FFT via `acfftExecC2C`
7. Compare with CPU DFT reference

## Key Concepts

ACFFT Library, Spectrum Analysis, Frequency Domain Filtering, Low-Pass Filter, DFT

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcMemcpy, hggcFree, hggcDeviceSynchronize

### ACFFT API
acfftPlan1d, acfftExecC2C, acfftDestroy

## Dependencies for Build/Run
ACFFT

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
