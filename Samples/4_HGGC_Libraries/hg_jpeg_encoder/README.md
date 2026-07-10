# hg_jpeg_encoder - JPEG Transcoder with Quality-Size Analysis

## Description

This sample decodes JPEG images and re-encodes them at multiple quality levels (30/50/70/90) to demonstrate the quality-size tradeoff. For each image, it reports the encoded size at each quality level alongside the original file size, then writes the highest-quality output to disk.

Pipeline per image:
1. Decode JPEG via `hgjpegDecode` → pixel data on device
2. For each quality level Q in {30, 50, 70, 90}:
   a. Set quality via `hgjpegEncoderParamsSetQuality`
   b. Encode via `hgjpegEncodeImage`
   c. Retrieve bitstream size via `hgjpegEncodeRetrieveBitstream`
3. Print quality-vs-size comparison table
4. Write highest-quality output to disk

## Key Concepts

HGGCJPEG Library, JPEG Encoding, Quality-Size Tradeoff, Transcoding, Image Compression

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## HGGC APIs Involved

### HGGC Runtime API
hggcMalloc, hggcFree, hggcDeviceSynchronize

### HGGCJPEG API
hgjpegCreate, hgjpegJpegStateCreate, hgjpegEncoderStateCreate, hgjpegEncoderParamsCreate, hgjpegEncoderParamsSetQuality, hgjpegEncoderParamsSetSamplingFactors, hgjpegGetImageInfo, hgjpegDecode, hgjpegEncodeImage, hgjpegEncodeRetrieveBitstream, hgjpegEncoderParamsDestroy, hgjpegEncoderStateDestroy, hgjpegJpegStateDestroy, hgjpegDestroy

## Dependencies for Build/Run
HGGCJPEG

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
