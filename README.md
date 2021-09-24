# DDS19 Emulation

This repository aims to emulate a specific audio effect device. The *DDS19* is a custom-built digital delay sampler with LFO modulation with quite a unique sound. The implementation combines traditional and deep learning techniques into a real-time [JUCE](https://juce.com) based audio plugin. 

There are three separate projects under this repository:

- **dds-nn** - Neural network training - PyTorch (Python)
- **dds-plugin** - AU and VST audio plugin implementation - JUCE (C++)
- **lstm-eigen** - LSTM inference - Eigen (C++)

The plugin implementation is still experimental and needs some work. The training code does not include a dataset as it was specifically designed for the device. However, some trained models are supplied with the plugin implementation. Those can also be used with the inference code.

