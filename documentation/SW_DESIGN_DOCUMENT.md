# SW Design Document

## 1. Purpose

This document describes the software design of the SWC DU/TU software located in this repository. It focuses on the implemented architecture, module responsibilities, runtime behavior, interfaces, configuration model, and key design constraints.

The software supports three runtime roles:

- `DUA`: Distribution Unit Alpha
- `DUB`: Distribution Unit Beta
- `TU`: Test Unit

## 2. Scope

The design covers the software under `DU/SW`, including:

- application entry and role selection
- command-manager hierarchy
- DU and TU hardware-management layers
- shared framework and message abstractions
- runtime communication with the test server, local modules, status emulator, and hardware devices

The design does not attempt to restate the FPGA register definitions or firmware implementation details contained in spreadsheets and XSA artifacts under `firmware/` and `documentation/`.

## 3. System Context

At runtime the software sits between:

- the test server, which sends commands and receives reports
- DU/TU local hardware, accessed through device abstractions and memory-mapped register interfaces
- the I/O module, accessed through Modbus TCP
- peer local software components, which exchange internal status over UDP
- an optional status emulator, which injects synthetic status data

High-level context:

```text
                +-------------------+
                |   Test Server     |
                | cmds / reports    |
                +---------+---------+
                          |
                    UDP command/report
                          |
        +-----------------+------------------+
        |                                    |
   +----v-----+                         +----v-----+
   |   DUA    |<----- UDP status ------ |   DUB    |
   | status   |                         | status   |
   | aggregate|                         | producer |
   +----+-----+                         +----------+
        ^
        |
        +----------- UDP status -----------+
                                            |
                                       +----v-----+
                                       |    TU    |
                                       | status   |
                                       | producer |
                                       +----------+
```

## 4. Design Drivers and Constraints

The implementation is shaped by the following constraints:

- direct interaction with hardware registers and interrupt devices
- event-driven operation rather than request-per-thread processing
- explicit support for three deployable module roles in a single codebase
- legacy C++ style and toolchain compatibility expectations
- limited build-system metadata in the repo beyond the SlickEdit project
- requirement to support both real hardware and emulator-driven status paths

Security and robustness constraints:

- external inputs from the test server, emulator, local UDP peers, and config files are untrusted
- status and command parsing must remain explicit and bounded
- operationally sensitive behavior such as shutdown, reboot, and hardware-control actions must remain centralized and reviewable

## 5. Source Organization

### 5.1 Directory Layout

- `common-src/common/`
  Shared managers and cross-cutting helpers such as config loading, SAP loading, scan-limit helpers, byte-order helpers, and the shared command-manager base classes.
- `common-src/framework/`
  Core event loop, device abstractions, timers, UDP networking, logging, timestamping, and utility infrastructure.
- `common-src/drivers/`
  Low-level device wrappers and register helpers for DU, TU, and UIO access.
- `common-src/messages/`
  Command and report message structures and serializers used for network communication.
- `du-app/`
  DU-specific command and hardware management for Alpha and Beta roles.
- `tu-app/`
  TU-specific command and hardware management.
- `config-files/`
  Runtime configuration, device mappings, logger settings, and startup recipe files.
- `documentation/`
  Supporting design notes and hardware register documents.
- `firmware/`
  FPGA/XSA and related hardware reference assets.

### 5.2 Key Entry Points

- `SwcAppInit.cpp`
  Process entry point. Selects `DUACmdMgr`, `DUBCmdMgr`, or `TUCmdMgr` based on the module argument.

## 6. Architectural Overview

The software follows a layered design:

1. Application-role layer
   `DUACmdMgr`, `DUBCmdMgr`, and `TUCmdMgr`
2. Shared command-processing layer
   `CmdMgrBase` and `DUCmdMgrBase`
3. Hardware-management layer
   `DUHWMgr`, `TUHWMgr`, `IOMHWMgr`
4. Framework/device layer
   `EventProcessor`, device abstractions, timers, UDP devices, logging
5. Message/model layer
   structures and message classes under `common-src/messages`

### 6.1 Role Selection

`main()` in `SwcAppInit.cpp` expects one module argument:

- `DUA`
- `DUB`
- `TU`

The selected role is instantiated and started immediately. The design assumes one process instance per module role.

### 6.2 Inheritance Structure

```text
EventProcessor
   |
   +-- CmdMgrBase
         |
         +-- DUCmdMgrBase
         |      |
         |      +-- DUACmdMgr
         |      +-- DUBCmdMgr
         |
         +-- TUCmdMgr
```

This structure centralizes shared event and command behavior while preserving role-specific hooks.

## 7. Major Components

### 7.1 EventProcessor

`EventProcessor` is the execution backbone of the system.

Responsibilities:

- maintain device-event registrations
- wait on registered file descriptors using `select()`
- dispatch callbacks when events occur
- provide a shared event loop for command managers

The design is callback driven. Concrete managers create devices, register callbacks with priorities, and then enter `EventProcessor::start()`.

### 7.2 CmdMgrBase

`CmdMgrBase` provides the common command and report interface shared by all module roles.

Responsibilities:

- create and own test-server UDP receive/transmit devices
- create and own the status-emulator UDP receive device
- parse incoming command messages
- validate command payloads
- dispatch role-specific handling for:
  - shutdown commands
  - steering commands
  - status-request commands
- send acknowledgements to the test server

Role-specific behavior is exposed through virtual hooks:

- `getCommandMgrName()`
- `handleSteeringCommand(...)`
- `handleStatusRequest(...)`
- `handleStatusEmulatorMessage(...)`

### 7.3 DUCmdMgrBase

`DUCmdMgrBase` extends `CmdMgrBase` with behavior common to DU Alpha and DU Beta.

Responsibilities:

- create and own DU UIO interrupt devices
- create and own the DU status timer
- handle shared DU steering behavior in test/offline conditions
- maintain last processed steering-word values
- implement the common DU scan-limit interrupt path
- implement common DU periodic status-refresh behavior
- optionally send processed steering-word reports

Role-specific DU behavior remains virtual:

- `handleDUStatusRequest(...)`
- `handlePendingDcuStatusAfterScanLimit()`
- `handlePreDcuStatusTimer()`
- `handlePostDcuStatusTimer(...)`
- `handleConfigInterruptRefresh()`
- `getProcessedKSineForReport()`

### 7.4 DUACmdMgr

`DUACmdMgr` is the primary SWC status aggregator.

Responsibilities:

- receive test-server commands on the Alpha network address
- receive internal status from DUB and TU on the local UDP status port
- initialize DU Alpha hardware access
- periodically refresh SWC and DU Alpha status
- service status-report requests from the test server
- convert aggregated local state into:
  - overall SWC reports
  - detailed SWC reports
  - detailed DCU reports

DUA-specific functions include:

- consuming `BETA_DCU_STATUS`, `BETA_DU_STATUS`, and `TU_STATUS`
- updating TWGS-facing DU/DCU status through `DUHWMgr`
- answering the richest set of status-request commands

### 7.5 DUBCmdMgr

`DUBCmdMgr` is the Beta DU status producer and forwarder.

Responsibilities:

- receive test-server commands on the Beta network address
- initialize DU Beta hardware access
- read Beta DU and DCU status
- forward Beta status to DUA over the local UDP status path
- support DU shared interrupt and timer behavior through `DUCmdMgrBase`

DUB does not provide the same detailed test-server reporting interface as DUA. Unsupported status requests are rejected.

### 7.6 TUCmdMgr

`TUCmdMgr` manages the Test Unit role.

Responsibilities:

- receive test-server commands on the TU network address
- initialize TU hardware access
- process TU steering commands when the command source is analog
- handle TU scan-limit and WLSP interrupts
- periodically send TU status to DUA over local UDP

TU inherits directly from `CmdMgrBase` because its interrupt/timer behavior is different from the DU pair.

### 7.7 DUHWMgr

`DUHWMgr` encapsulates DU hardware state and status aggregation.

Key responsibilities:

- initialize DU hardware access and timing parameters
- read and write DU registers
- manage steering-word and board-control operations
- read SWC, DU, and DCU status
- validate DCU status
- queue DCU status updates for forwarding/reporting
- aggregate Alpha, Beta, TU, power, ATB, and temperature status into SWC-level status
- optionally substitute emulator-provided status

This class also provides the in-memory state used by DUA for overall reporting.

### 7.8 TUHWMgr

`TUHWMgr` encapsulates TU hardware access.

Responsibilities:

- initialize TU hardware
- read TU register-level status
- control RLTD, RLCP, and RLSC signaling
- provide TU scan-limit status
- optionally substitute emulator-provided TU status

### 7.9 IOMHWMgr

`IOMHWMgr` provides Modbus TCP access to the I/O module.

Responsibilities:

- create and maintain a Modbus TCP context
- read discrete I/O input state
- expose I/O module health data used by the DU status layer

## 8. Communication Interfaces

### 8.1 Test Server Interface

The test server communicates with each runtime role over UDP using the message types defined in `SWCMsgTypes.h` and the corresponding message classes.

Inbound command message IDs:

- `STEERING_CMD_MSG_ID`
- `SHUTDOWN_CMD_MSG_ID`
- `STATUS_REQUEST_CMD_MSG_ID`

Outbound report/ack IDs:

- `SWC_OVERALL_STATUS_RPT_MSG_ID`
- `SWC_DETAILED_STATUS_RPT_MSG_ID`
- `DCU_DETAILED_STATUS_RPT_MSG_ID`
- `SWC_ACK_RPT_MSG_ID`
- `SWC_PROCESSED_SW_RPT_MSG_ID`

### 8.2 Internal Module Interface

Internal software components exchange status using UDP and the following internal message IDs:

- `BETA_DCU_STATUS`
- `BETA_DU_STATUS`
- `TU_STATUS`

Direction of flow:

- DUB -> DUA
  - Beta DU status
  - Beta DCU status
- TU -> DUA
  - TU status

DUA is therefore the central status consumer for peer-module state.

### 8.3 Status Emulator Interface

Each command manager also opens a UDP input for emulator status data.

The code recognizes emulator message categories such as:

- SWCR status
- DUA status
- DUB status
- TU status
- DCU status

Each module handles only the subset relevant to its role.

### 8.4 Hardware Device Interfaces

The design uses:

- `UDPNetworkDevice`
  for network command, report, and internal status traffic
- `UIODevice`
  for interrupt-driven hardware notification
- `TimerDevice`
  for periodic status refresh
- `DUDevice`
  for DU hardware register access
- `TUDevice`
  for TU hardware register access

## 9. Runtime Behavior

### 9.1 Startup Sequence

Common startup pattern:

1. load configuration
2. load SAP data where applicable
3. initialize logging
4. create shared command UDP endpoints
5. create role-specific local UDP endpoints
6. initialize hardware managers
7. create interrupt and timer devices
8. send `InitCompleteAck`
9. enter the event loop

Role-specific differences:

- DUA creates a local UDP server to receive DUB/TU status
- DUB creates a local UDP client to send status to DUA
- TU creates a local UDP client to send status to DUA
- DUA and DUB share DU interrupt/timer setup through `DUCmdMgrBase`
- TU sets up its own interrupt and timer devices directly

### 9.2 Command Processing

`CmdMgrBase::processTestServerMsg()` performs the common sequence:

1. read UDP payload
2. validate minimum header size
3. byte-swap the message header to local format
4. validate the header and record payload
5. dispatch to the appropriate command-specific parser
6. invoke the role-specific handler

Shared design benefit:

- message validation is centralized
- shutdown handling is centralized
- role-specific logic only receives already-decoded payload structures

### 9.3 Steering Command Flow

DU path:

- handled centrally in `DUCmdMgrBase`
- active only when the SWC/OLTE mode is offline or force-test mode is enabled
- applies Alpha/Beta K-sine values through `DUHWMgr`
- optionally triggers calibration/boresight DCU actions
- toggles the DU software trigger

TU path:

- handled in `TUCmdMgr`
- active when the command source is analog
- applies Alpha/Beta K-sine values through `TUHWMgr`
- pulses RLCP/RLSC as requested
- toggles RLTD to start steering processing

### 9.4 DU Scan-Limit Flow

DU scan-limit interrupt processing is shared in `DUCmdMgrBase`.

Sequence:

1. acknowledge UIO interrupt
2. read ARM and ATB Alpha/Beta K-sine values
3. sign-extend 10-bit values
4. run software scan-limit checks
5. read firmware scan-limit status
6. store the last processed Alpha/Beta values
7. perform role-specific post-scan-limit handling
8. optionally send processed steering-word reports

Role-specific post-processing:

- DUA updates TWGS-facing DCU status from the queue
- DUB forwards queued DCU status to DUA

### 9.5 Periodic Status Flow

DUA:

- alternates between custom, I/O-module, and config SWC status reads
- refreshes Alpha DU status
- periodically reads DCU status, with full-list refresh every configured interval
- keeps SWC aggregate state available for reporting

DUB:

- refreshes Beta DU status
- forwards Beta DU status to DUA
- reads DCU status and forwards dequeued DCU entries to DUA

TU:

- refreshes TU status
- forwards TU status to DUA

### 9.6 DUA Status Aggregation Flow

DUA is the only module that combines:

- local Alpha DU status
- local SWC aggregate state
- Beta DU/DCU status received from DUB
- TU status received from TU

This aggregation is then used to answer:

- overall SWC status requests
- detailed SWC status requests
- detailed Alpha/Beta DCU status requests

## 10. Data Model

### 10.1 Command Payloads

Key command payloads:

- `ShutdownCmdDataType`
- `SteeringCmdDataType`
- `StatusRequestCmdDataType`

### 10.2 Status Payloads

Key status structures:

- `SWCOverallStatusDataType`
- `SWCDetailedStatusDataType`
- `DCUStatus`
- `DUTUStatusType`
- `BetaDCUStatusMsg`
- `DUTUStatusMsg`

### 10.3 Configuration Model

Primary configuration comes from `config-files/config.txt`.

Important settings include:

- IP addresses
- test-server and logger ports
- local inter-module status port
- status-emulator port
- status timer interval
- DU refresh interval
- force-test-mode flag
- DCU SPI delays
- scan-limit center-frequency selection
- TU pulse durations
- DCU firmware version expectations

The design assumes configuration values are loaded at startup and then consumed by the role-specific managers and hardware managers.

## 11. Error Handling and Logging

The codebase uses a shared `Logger` singleton and direct `printf()` diagnostics.

Current error-handling pattern:

- return `ERROR` from startup/device initialization failures
- log device open and event-registration failures
- reject invalid command/message payloads
- continue event-loop processing when recoverable read failures occur

Observations from the current design:

- diagnostics are duplicated between `printf()` and logger calls
- some communication paths log errors but do not escalate them
- error propagation is primarily immediate-return based during startup and local logging during runtime

## 12. Security Considerations

Security-relevant design properties:

- external network packets are parsed through typed message wrappers
- role-specific command behavior is centralized behind the command-manager layer
- hardware-control operations remain explicit and narrow
- emulator support is isolated to dedicated UDP input channels

Security considerations for maintainers:

- treat config values and UDP payloads as untrusted input
- preserve explicit bounds and validation on message decoding
- avoid widening hardware-control behavior without centralized review
- be cautious with shutdown/reboot behavior, because it is implemented in shared infrastructure

## 13. Build and Deployment

The checked-in build descriptor is `swc-esw-src.vpj`.

Build characteristics:

- SlickEdit project-driven build
- Debug and Release configurations
- compile/link targets based on `cc`
- recursive inclusion of project source files through `*.c`, `*.cpp`, and `*.h`

Deployment assumption:

- the same software tree is built once and launched with a module-role argument to select DUA, DUB, or TU behavior

## 14. Current Design Strengths

- clear runtime-role separation
- centralized command decoding and validation
- shared DU behavior extracted into a dedicated DU base class
- DUA-centered status aggregation keeps reporting logic in one place
- event-driven architecture maps well to network, timer, and interrupt inputs

## 15. Current Design Limitations

- configuration loading logic is still repeated across the three concrete command managers
- DUA is a central aggregation point, so visibility of overall SWC state depends on DUA availability
- the codebase mixes framework-style logging and direct console printing
- the design is strongly coupled to current device interfaces and register layouts
- automated verification infrastructure is not present in the repository

## 16. Recommended Evolution Areas

Based on the implemented structure, the next logical design improvements are:

- move repeated command-manager startup configuration loading into a shared configuration object or helper
- harden network-input validation uniformly across all UDP receive paths
- continue reducing duplicated status-forwarding/reporting boilerplate where behavior is truly common
- document shutdown semantics explicitly, especially for the accepted shutdown command variants

## 17. Summary

This project implements an event-driven SWC control application with three deployable roles. The software uses shared framework and message layers, DU/TU-specific hardware managers, and a command-manager hierarchy that now separates:

- common command behavior
- common DU behavior
- role-specific application behavior

The design is pragmatic, explicit, and hardware oriented. DUA acts as the aggregation and reporting center, while DUB and TU act primarily as role-specific control and status producers within the larger SWC runtime.
