# acdnn_conv_activation - ACDNN Convolution + ReLU Pipeline

## Description

This sample uses the ACDNN library to build a classic CNN forward computation pipeline: 2D convolution + ReLU activation.

Workflow:
1. Create input tensor (NCHW, FP32) and convolution filter (KCRS), initialized with random data.
2. Set up ACDNN tensor descriptor, filter descriptor, convolution descriptor, and activation descriptor.
3. Query convolution workspace size and execute forward convolution (`acdnnConvolutionForward`).
4. Apply ReLU activation to convolution output (`acdnnActivationForward`).
5. Copy results back to host and print.
6. Implement reference convolution + ReLU on host using naive loops, verify PPU results.

## Key Concepts

Deep Learning, Convolution, Activation Function, ACDNN Library.

## Command-Line Arguments

| Argument | Description | Default |
| --- | --- | --- |
| -N=<int> | batch size | 1 |
| -C=<int> | input channels | 4 |
| -H=<int> | input height | 8 |
| -W=<int> | input width | 8 |
| -K=<int> | output channels | 8 |
| -R=<int> | kernel height | 3 |
| -S=<int> | kernel width | 3 |
| -device=<id> | specify Zhenwu PPU device | default device |

## Supported Operating Systems

Linux

## Supported CPU Architectures

x86_64

## Supported PPU Architectures

ppu001, ppu0015

## ACDNN APIs Involved

### ACDNN API
acdnnCreate, acdnnDestroy, acdnnSetStream, acdnnCreateTensorDescriptor, acdnnSetTensor4dDescriptor, acdnnDestroyTensorDescriptor, acdnnCreateFilterDescriptor, acdnnSetFilter4dDescriptor, acdnnDestroyFilterDescriptor, acdnnCreateConvolutionDescriptor, acdnnSetConvolution2dDescriptor, acdnnDestroyConvolutionDescriptor, acdnnGetConvolutionForwardWorkspaceSize, acdnnConvolutionForward, acdnnCreateActivationDescriptor, acdnnSetActivationDescriptor, acdnnDestroyActivationDescriptor, acdnnActivationForward

### HGGC Runtime API
hggcMalloc, hggcFree, hggcMemcpy, hggcStreamCreate, hggcStreamDestroy, hggcDeviceSynchronize.

## Dependencies for Build/Run
ACDNN

## Prerequisites
Please download and install the T-Head SAIL toolkit for your platform.
