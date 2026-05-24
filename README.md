# Mitsubishi CNC M70 Ethernet Communication Library

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)

English | [简体中文](README.zh-CN.md)

## Author and Copyright

- Copyright (c) 2022-2026 wqliceman
- GitHub: iceman
- Email: wqliceman@gmail.com
- License: MIT, see [LICENSE](LICENSE)

## Contents

- Overview
- Project Status
- Repository Layout
- Build
- Quick Start
- API Summary
- Safe String API (_ex)
- Logging and Error Handling
- Compatibility Notes
- API Index by Business Scenario
- Troubleshooting
- Release Notes Template
- License

## Overview

This project is a C library for Ethernet communication with Mitsubishi CNC controllers (focused on M70, EZSocket protocol).

It provides:
- Connection management
- Structured data read/write APIs
- Logging and error reporting utilities
- Cross-platform support for Windows and Linux

## Project Status

- Language: C
- Primary protocol: EZSocket
- Tested target: Mitsubishi M70
- Build outputs: executable test app by default
- Windows runtime: Winsock is initialized and cleaned up inside the library

## Repository Layout

- `mitsubishi_cnc_m70_ezsocket_net/`: core library source and test entry (`main.c`)
- `README.md`: English documentation
- `README.zh-CN.md`: Chinese documentation
- `makefile`, `common.mk`, `config.mk`: GNU Make build entry/config
- `LICENSE`: MIT license

## Build

### Linux / WSL (GNU Make)

From repository root:

```bash
make clean
make
```

Default target now builds:
- `build/lib/libm70_ezsocket.a`
- `build/lib/libm70_ezsocket.so` on POSIX/GCC toolchains
- `build/bin/mitsubishi_cnc_m70_test`

Additional targets:

```bash
make static
make shared
make example
make test
```

`make test` runs a socket regression test that covers fragmented reads and incomplete-response disconnect handling.

### Windows

You can use:
- Visual Studio solution in `mitsubishi_cnc_m70_ezsocket_net/mitsubishi_cnc_m70_ezsocket_net.sln`
- Or build library/example/test targets via WSL using GNU Make

The Visual Studio project currently remains an example application project. The dedicated static/shared library outputs are provided by the GNU Make flow above.

## Quick Start

Include:

```c
#include "m70_ezsocket.h"
```

Minimal example:

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

For a complete, maintained workflow example, see:
- `mitsubishi_cnc_m70_ezsocket_net/main.c`

## API Summary

### Connection

```c
bool m70_cnc_connect(const char* ip_addr, int port, m70_nc_type_e type, m70_conn_t* conn);
void m70_cnc_disconnect(m70_conn_t* conn);
```

### Common Read APIs

Representative functions:
- `m70_cnc_read_status`
- `m70_cnc_read_system_count`
- `m70_cnc_read_nc_axis_count`
- `m70_cnc_read_axis_position`
- `m70_cnc_read_spindle_speed`
- `m70_cnc_read_feed_speed`
- `m70_cnc_read_alarm`

### Common Write APIs

```c
m70_error_code_e m70_cnc_write_common_variable(m70_conn_t* conn, short system_no, uint32 index, double value);
m70_error_code_e m70_cnc_write_common_variable_comment(m70_conn_t* conn, uint32 index, char* value);
```

See full API declarations in:
- `mitsubishi_cnc_m70_ezsocket_net/m70_ezsocket.h`

## Safe String API (_ex)

The `_ex` variants accept explicit buffer length arguments and are recommended for all new code.

Examples:

```c
m70_cnc_read_nc_version_ex(conn, version, version_len);
m70_cnc_read_main_program_name_ex(conn, system_no, type, prog, prog_len);
m70_cnc_read_axis_name_ex(conn, system_no, names, names_len, &axis_count);
m70_cnc_write_common_variable_comment_ex(conn, index, text, text_len);
```

Legacy APIs are retained for backward compatibility and internally delegate to safer handling paths.

## Logging and Error Handling

- On Windows, callers do not need to invoke `WSAStartup`/`WSACleanup`; the library manages socket runtime internally.

This release also hardens frame reads on the TCP layer so GIOP/EZSocket responses are drained to the requested length instead of assuming a single `recv` call returns the full payload.

- Logging module: `mitsubishi_cnc_m70_ezsocket_net/m70_log.h`
- Error module: `mitsubishi_cnc_m70_ezsocket_net/m70_error.h`

Recommended pattern:

```c
#define SAFE_CALL(expr)                                \
    do {                                               \
        m70_error_code_e _ret = (expr);                \
        if (_ret != M70_ERROR_CODE_OK) goto cleanup;   \
    } while (0)
```

## Compatibility Notes

- Configure CNC Ethernet and protocol settings before connecting.
- Some register/section availability may vary by controller model and configuration.
- Validate machine-side data mapping before deploying to production.

## API Index by Business Scenario

### Status and Device Overview

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

### Axis and Position

- `m70_cnc_read_axis_name_ex`
- `m70_cnc_read_axis_position`
- `m70_cnc_read_all_axis_position`
- `m70_cnc_read_svo_load`

### Spindle and Feed

- `m70_cnc_read_spindle_speed`
- `m70_cnc_read_spindle_override`
- `m70_cnc_read_spindle_load`
- `m70_cnc_read_feed_speed`
- `m70_cnc_read_feed_override`

### Program 

- `m70_cnc_read_main_program_name_ex`
- `m70_cnc_read_sub_program_name_ex`
- `m70_cnc_read_program_file_info`
- `m70_cnc_read_program_block`

### Alarm and Runtime Time Counters

- `m70_cnc_read_alarm`
- `m70_cnc_read_is_alarm`
- `m70_cnc_read_power_on_time`
- `m70_cnc_read_auto_operation_time`
- `m70_cnc_read_auto_startup_time`
- `m70_cnc_read_cycle_time`
- `m70_cnc_read_external_accumulative_time`
- `m70_cnc_read_cutting_time`
- `m70_cnc_read_system_datetime`

## Troubleshooting

### 1. Connection Failed

Symptoms:
- `m70_cnc_connect` returns `false`
- Connection-related logs in `m70_log`

Checklist:
- Verify CNC IP, port, and model type (`m70_nc_type_e`)
- Confirm CNC Ethernet module and EZSocket service are enabled
- Verify host-to-CNC network reachability (same subnet/routing/firewall)
- Check that no other client has exhausted controller connection resources

### 2. Timeout / Slow Response

Symptoms:
- Read API intermittently fails
- High latency or unstable polling loop

Checklist:
- Reduce polling frequency and group high/low frequency metrics separately
- Avoid reading large text/program blocks in high-frequency loops
- Check network quality (packet loss, jitter, duplex mismatch)
- Use `_ex` APIs to avoid downstream string handling overhead/risk

### 3. Read/Write Failed

Symptoms:
- API returns non-`M70_ERROR_CODE_OK`
- Unexpected empty strings or zero values

Checklist:
- Validate `system_no`, axis index, and variable index ranges
- Confirm target register/section is available on current machine configuration
- Validate buffer sizes in caller for string outputs (`*_ex` APIs)
- Inspect latest error/log details from `m70_error` and `m70_log`

## Release Notes Template

Use this template for each release entry (for example in GitHub Releases or a `CHANGELOG.md` file):

```markdown
## [vX.Y.Z] - YYYY-MM-DD

### Added
- 

### Changed
- 

### Fixed
- 

### Removed
- 

### Compatibility
- API compatibility: [backward-compatible / breaking]
- Device/firmware notes:

### Migration Notes
- 
```

## License

This project is licensed under the MIT License. See `LICENSE`.