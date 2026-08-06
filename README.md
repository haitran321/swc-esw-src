# SWC DU/TU Embedded Software Overview

## Purpose

This project contains the C++ software for the SWC Distribution Unit environment. It runs in one of three module roles:

- `DUA`: DU Alpha controller
- `DUB`: DU Beta controller
- `TU`: Test Unit controller

The executable entry point is `SwcAppInit.cpp`. At startup it selects the correct command manager based on the module type argument and starts the corresponding event-driven application.

## High-Level Architecture

The software is organized around command managers that receive commands from a test server, interact with local hardware devices, and report status back over UDP.

- `DUACmdMgr`
  Handles the DU Alpha role, maintains local SWC status, receives forwarded Beta DU/DCU and TU status, and sends detailed status reports to the test server.
- `DUBCmdMgr`
  Handles the DU Beta role, reads local Beta DU/DCU hardware status, and forwards those updates to DU Alpha.
- `TUCmdMgr`
  Handles the TU role, processes steering-related TU hardware actions, and forwards TU status to DU Alpha.

Shared command-processing logic is centralized in:

- `common-src/common/CmdMgrBase.*`
  Shared command socket setup, command decoding, status-emulator input, and test-server acknowledgements.
- `du-app/DUCmdMgrBase.*`
  Shared DU-specific behavior such as DU interrupt setup, steering command handling, DU status timers, and processed steering-word reporting.

## Main Components

### Shared Source

`common-src/` contains code shared across all module types.

- `common/`
  Common managers and utilities such as config loading, SAP data loading, scan-limit helpers, byte-order helpers, and shared command-manager base classes.
- `framework/`
  Event loop, device abstractions, timers, UDP networking, logging, and utility support classes.
- `drivers/`
  Hardware-facing device wrappers for DU, TU, UIO, and low-level register helpers.
- `messages/`
  Command and report message definitions used for communication with the test server and local modules.

### DU-Specific Source

`du-app/` contains DU hardware managers and DU command managers.

- `DUHWMgr`
  DU hardware access and status aggregation.
- `IOMHWMgr`
  I/O module status access over Modbus TCP.
- `DUACmdMgr`
  Alpha DU application logic.
- `DUBCmdMgr`
  Beta DU application logic.
- `DUCmdMgrBase`
  Shared DU command-manager behavior.

### TU-Specific Source

`tu-app/` contains TU command and hardware management.

- `TUHWMgr`
  TU hardware access and control.
- `TUCmdMgr`
  TU application logic.

## Runtime Model

The applications are event driven.

- UDP sockets are used to receive commands from the test server.
- UDP sockets are used to send reports and acknowledgements back to the test server.
- DUA also receives forwarded status from DUB and TU over a local UDP channel.
- UIO devices are used for hardware interrupt notification.
- Timer devices trigger periodic status reads and refresh behavior.

Typical responsibilities at runtime:

- process shutdown, steering, and status request commands
- read hardware status registers
- perform scan-limit checks
- maintain DU/TU/DCU status state
- forward local status between modules when required
- send status reports and acknowledgements to the test server

## Entry Point

`SwcAppInit.cpp` expects a single module-type argument:

- `DUA`
- `DUB`
- `TU`

Based on that argument it constructs and starts one of:

- `DUACmdMgr`
- `DUBCmdMgr`
- `TUCmdMgr`

## Configuration

Runtime configuration is primarily driven by `config-files/config.txt`.

Important configuration areas include:

- IP addresses for the test server, logger, Alpha DU, Beta DU, TU, and I/O module
- UDP port assignments for command, status, logger, and emulator traffic
- status refresh intervals
- force-test-mode control
- DCU SPI timing values
- scan-limit settings
- TU pulse timing
- DCU firmware version expectations

Additional configuration and startup artifacts live under `config-files/`, including:

- `devices.conf`
- `logger.conf`
- `sap.txt`
- `myapp-init/`

## Build and Development

The checked-in build description is the SlickEdit project file `swc-esw-src.vpj`.

Configured targets include:

- Debug compile and link
- Release compile and link
- workspace build and rebuild via `vsbuild`

The project does not include a repo-local CMake or Make build system.

## Hardware and Supporting Assets

- `firmware/`
  Reference FPGA/XSA artifacts and register-definition documents for DU, TU, and emulator environments.
- `documentation/`
  Supporting notes and register-definition spreadsheets.
- `libmodbus/stage/`
  Staged Modbus headers and libraries used by the I/O module integration.

## Security and Operational Notes

- Treat all network input, emulator input, and configuration values as untrusted.
- Hardware access is performed through device abstractions and memory-mapped register interfaces, so changes should remain explicit and easy to review.
- DUA is the main aggregation point for overall system status visibility.
- DUB and TU primarily provide role-specific control and status forwarding into the larger SWC status flow.
