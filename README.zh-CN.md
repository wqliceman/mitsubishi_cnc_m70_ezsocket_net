# 三菱 CNC M70 以太网通信库

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

[English](README.md) | 简体中文

## 作者与版权信息

- 版权所有 Copyright (c) 2022-2026 wqliceman
- GitHub 用户名：iceman
- 邮箱：wqliceman@gmail.com
- 许可证：MIT，详见 [LICENSE](LICENSE)

## 目录

- 项目简介
- 当前状态
- 目录说明
- 构建方式
- 快速开始
- API 概览
- 安全字符串接口（_ex）
- 日志与错误处理
- 兼容性说明
- 按业务场景分类的 API 索引
- 常见故障排查
- 版本变更记录模板
- 许可证

## 项目简介

本项目是一个基于 C 语言的三菱 CNC 控制器以太网通信库（当前重点支持 M70，协议为 EZSocket）。

主要提供：
- 连接管理能力
- 结构化数据读写接口
- 日志与错误处理组件
- Windows / Linux 跨平台支持

## 当前状态

- 开发语言：C
- 核心协议：EZSocket
- 已验证设备：Mitsubishi M70
- 默认构建产物：测试可执行程序
- Windows 运行时：库内部自动完成 Winsock 初始化与清理

## 目录说明

- `mitsubishi_cnc_m70_ezsocket_net/`：核心源码与测试入口（`main.c`）
- `README.md`：英文文档
- `README.zh-CN.md`：中文文档
- `makefile`、`common.mk`、`config.mk`：GNU Make 构建入口与配置
- `LICENSE`：MIT 许可证

## 构建方式

### Linux / WSL（GNU Make）

在仓库根目录执行：

```bash
make clean
make
```

默认会生成 `mitsubishi_cnc_test`。

### Windows

可选两种方式：
- 使用 `mitsubishi_cnc_m70_ezsocket_net/mitsubishi_cnc_m70_ezsocket_net.sln`（Visual Studio）
- 使用 WSL 执行 GNU Make

## 快速开始

头文件：

```c
#include "m70_ezsocket.h"
```

最小示例：

```c
#include <stdio.h>
#include "m70_ezsocket.h"

int main(void)
{
    m70_conn_t conn = {0};
    if (!m70_cnc_connect("192.168.123.130", 683, EZNC_SYS_MELDAS700M, &conn)) {
        return -1;
    }

    char nc_version[128] = {0};
    if (m70_cnc_read_nc_version_ex(&conn, nc_version, sizeof(nc_version)) == M70_ERROR_CODE_OK) {
        printf("NC Version: %s\n", nc_version);
    }

    m70_cnc_disconnect(&conn);
    return 0;
}
```

完整且持续维护的流程示例请参考：
- `mitsubishi_cnc_m70_ezsocket_net/main.c`

## API 概览

### 连接接口

```c
bool m70_cnc_connect(const char* ip_addr, int port, m70_nc_type_e type, m70_conn_t* conn);
void m70_cnc_disconnect(m70_conn_t* conn);
```

### 常用读取接口

代表性函数：
- `m70_cnc_read_status`
- `m70_cnc_read_system_count`
- `m70_cnc_read_nc_axis_count`
- `m70_cnc_read_axis_position`
- `m70_cnc_read_spindle_speed`
- `m70_cnc_read_feed_speed`
- `m70_cnc_read_alarm`

### 常用写入接口

```c
m70_error_code_e m70_cnc_write_common_variable(m70_conn_t* conn, short system_no, uint32 index, double value);
m70_error_code_e m70_cnc_write_common_variable_comment(m70_conn_t* conn, uint32 index, char* value);
```

完整 API 声明见：
- `mitsubishi_cnc_m70_ezsocket_net/m70_ezsocket.h`

## 安全字符串接口（_ex）

推荐新代码优先使用 `_ex` 版本（显式传入缓冲区长度），降低越界风险。

示例：

```c
m70_cnc_read_nc_version_ex(conn, version, version_len);
m70_cnc_read_main_program_name_ex(conn, system_no, type, prog, prog_len);
m70_cnc_read_axis_name_ex(conn, system_no, names, names_len, &axis_count);
```

旧接口仍保留以兼容历史代码。

## 日志与错误处理

- Windows 下调用方无需再手工调用 `WSAStartup`/`WSACleanup`，库会自行管理 socket 运行时。
- TCP 层现在会把协议响应读满目标长度，避免把一次 `recv` 误当成完整 GIOP/EZSocket 帧。
- 日志模块：`mitsubishi_cnc_m70_ezsocket_net/m70_log.h`
- 错误模块：`mitsubishi_cnc_m70_ezsocket_net/m70_error.h`

建议统一调用检查：

```c
#define SAFE_CALL(expr)                                \
    do {                                               \
        m70_error_code_e _ret = (expr);                \
        if (_ret != M70_ERROR_CODE_OK) goto cleanup;   \
    } while (0)
```

## 兼容性说明

- 连接前请先完成 CNC 端以太网与协议参数配置。
- 不同机型/配置下，寄存器分区与可访问数据可能存在差异。
- 生产环境接入前请先完成机台侧地址映射验证。

## 按业务场景分类的 API 索引

### 状态与设备概览

- `m70_cnc_read_status`
- `m70_cnc_read_system_count`
- `m70_cnc_read_nc_axis_count`
- `m70_cnc_read_all_axis_count`
- `m70_cnc_read_spindle_axis_count`
- `m70_cnc_read_plc_axis_count`
- `m70_cnc_read_nc_type`
- `m70_cnc_read_nc_version_ex`
- `m70_cnc_read_nc_name_version_ex`
- `m70_cnc_read_plc_version_ex`

### 轴与位置

- `m70_cnc_read_axis_name_ex`
- `m70_cnc_read_axis_position`
- `m70_cnc_read_all_axis_position`
- `m70_cnc_read_svo_load`

### 主轴与进给

- `m70_cnc_read_spindle_speed`
- `m70_cnc_read_spindle_override`
- `m70_cnc_read_spindle_load`
- `m70_cnc_read_feed_speed`
- `m70_cnc_read_feed_override`

### 程序

- `m70_cnc_read_main_program_name_ex`
- `m70_cnc_read_sub_program_name_ex`
- `m70_cnc_read_program_file_info`
- `m70_cnc_read_program_block`

### 报警与时间统计

- `m70_cnc_read_alarm`
- `m70_cnc_read_is_alarm`
- `m70_cnc_read_power_on_time`
- `m70_cnc_read_auto_operation_time`
- `m70_cnc_read_auto_startup_time`
- `m70_cnc_read_cycle_time`
- `m70_cnc_read_external_accumulative_time`
- `m70_cnc_read_cutting_time`
- `m70_cnc_read_system_datetime`

## 常见故障排查

### 1. 连接失败

现象：
- `m70_cnc_connect` 返回 `false`
- 日志中出现连接相关错误

排查项：
- 核对 CNC IP、端口、机型参数（`m70_nc_type_e`）
- 确认 CNC 端以太网模块与 EZSocket 服务已开启
- 检查主机到 CNC 网络可达性（同网段/路由/防火墙）
- 检查控制器连接资源是否已被其他客户端占用

### 2. 超时或响应慢

现象：
- 读取接口偶发失败
- 轮询周期中出现明显延迟波动

排查项：
- 降低轮询频率，按高频/低频指标分层采集
- 避免在高频循环中读取大文本或程序块
- 检查网络质量（丢包、抖动、双工不匹配）
- 优先使用 `_ex` 接口，降低字符串处理风险

### 3. 读写失败

现象：
- 接口返回非 `M70_ERROR_CODE_OK`
- 文本为空或数值异常为 0

排查项：
- 检查 `system_no`、轴号、变量索引范围是否合法
- 确认当前机型配置下目标分区/地址可访问
- 校验调用方缓冲区长度（字符串接口优先 `_ex`）
- 结合 `m70_error` 与 `m70_log` 查看最近一次错误信息

## 版本变更记录模板

建议每次发布使用以下模板（可用于 GitHub Releases 或 `CHANGELOG.md`）：

```markdown
## [vX.Y.Z] - YYYY-MM-DD

### 新增 Added
- 

### 变更 Changed
- 

### 修复 Fixed
- 

### 移除 Removed
- 

### 兼容性 Compatibility
- API 兼容性: [backward-compatible / breaking]
- 设备/固件说明:

### 迁移说明 Migration Notes
- 
```

## 许可证

本项目基于 MIT 许可证发布，详见 `LICENSE`。
