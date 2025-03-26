# 三菱 CNC M70 以太网通信库

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

## 概述

本项目提供了一个C语言库，用于通过EZSocket协议与三菱CNC M70系列机床进行以太网通信。它使开发人员能够轻松地与CNC机床进行交互，实现数据采集、监控和控制等功能。

## 特性

- **跨平台**: 支持Windows和Linux操作系统
- **协议实现**: 实现了EZSocket协议，确保通信的顺畅
- **数据采集**: 提供API来读取各种机床数据，如状态、轴位置、主轴转速等
- **可扩展性**: 易于扩展以支持其他CNC模型和数据点

## 快速开始

### 环境要求

- **以太网配置**: 确保三菱CNC M70机床上的以太网模块已正确配置
- **开发环境**: C编译器（如GCC、MSVC）和必要的套接字编程库