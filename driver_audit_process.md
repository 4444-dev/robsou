# Signed Kernel Driver Auditing & Discovery Process

This document outlines the standard engineering workflow used by security researchers to discover, triage, and verify signed Windows kernel drivers (`.sys`) that expose arbitrary memory, I/O, or process primitives—commonly referred to in research as Bring Your Own Vulnerable Driver (BYOVD) analysis—and why anti-cheat and EDR systems detect them.

---

## 1. Corpus Collection (Sourcing Candidates)

Public repositories like **LOLDrivers** and Microsoft's **Vulnerable Driver Blocklist** (`DriverSiPolicy.p7b`) already catalog widely known drivers. Researching previously uncatalogued candidates starts with gathering a clean corpus of signed `.sys` binaries:

- **OEM & Hardware Vendor Packages:** Motherboard utilities (RGB lighting, fan control, overclocking, BIOS flashers), laptop power-management suites, peripheral software (mice, keyboards, audio interfaces), and enterprise diagnostics. These frequently ship legacy WDM drivers with broad hardware-access IOCTLs.
- **Microsoft Update Catalog:** Searching by hardware class or vendor INF packages yields WHQL-signed drivers that are often overlooked by public blocklists.
- **Signature & Blocklist Pre-Filtering:**
  - Verify the Authenticode signature (`signtool verify /kp` or PowerShell `Get-AuthenticodeSignature`). Check whether the certificate is valid, expired (timestamped signatures can still load depending on OS policy), or revoked.
  - Cross-reference SHA-256 hashes and signer thumbprints against `LOLDrivers` and Microsoft's `DriverSiPolicy.p7b` to filter out already-known entries.
  - Check HVCI (Memory Integrity) compatibility if targeting modern Windows 11 configurations, as HVCI enforces strict section alignment and NX/non-executable data constraints.

---

## 2. Automated Static Triage

Manually reversing thousands of `.sys` files is impractical. Researchers use automated PE scanners (Python `pefile`, YARA, or Ghidra/IDA headless scripts) to score candidates by imported APIs and device security descriptors.

### High-Signal Kernel Imports
A driver exposing useful primitives typically imports one or more of the following from `ntoskrnl.exe` or `hal.dll`:

| Category | Key Imports / Instructions | Primitive Provided |
|---|---|---|
| **Physical Memory** | `MmMapIoSpace`, `MmMapIoSpaceEx`, `ZwMapViewOfSection` (`\Device\PhysicalMemory`), `MmCopyMemory` | Map or read/write arbitrary physical RAM |
| **Virtual / Process Memory** | `MmCopyVirtualMemory`, `ZwReadVirtualMemory`, `ZwWriteVirtualMemory`, `ZwOpenProcess` | Cross-process memory access or handle elevation |
| **Hardware / MSR / Port I/O** | `__readmsr` / `__writemsr` (`rdmsr`/`wrmsr`), `HalGetBusData`, `HalSetBusData`, `WRITE_PORT_UCHAR` | CPU model-specific register or PCI config manipulation |
| **Execution / Allocation** | `ExAllocatePool` / `ExAllocatePool2`, `MmAllocateContiguousMemory`, `ZwLoadDriver` | Kernel pool allocation or secondary driver loading |

### Security Descriptor Triage
Even if a driver exposes `MmMapIoSpace`, it is far less accessible if restricted to `SYSTEM` or Administrators. Scanners check for:
- `IoCreateDevice` (often defaults to permissive access depending on device type and INF settings) vs. `IoCreateDeviceSecure` with a restrictive SDDL string (e.g., `D:P(A;;GA;;;SY)(A;;GA;;;BA)`).
- Calls to `SeSinglePrivilegeCheck` or `SeAccessCheck` inside the dispatch handler.

---

## 3. Reverse Engineering the Dispatch Routine

Once triage narrows the list, each candidate is loaded into **IDA Pro**, **Ghidra**, or **Binary Ninja** with kernel symbols/types.

1. **Locate `DriverEntry`:**
   - Trace `DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]` (offset `0xE0` on x64, index `0x0E`).
   - Also check `MajorFunction[IRP_MJ_CREATE]` (index `0x00`) for caller validation (e.g., checking `SeSinglePrivilegeCheck`, token elevation, or specific process names).

2. **Map `IoControlCode` Handlers:**
   - Inside the `IRP_MJ_DEVICE_CONTROL` handler, locate `Irp->Tail.Overlay.CurrentStackLocation->Parameters.DeviceIoControl.IoControlCode`.
   - Decode each `CTL_CODE` (`DeviceType`, `Function`, `Method`, `Access`):
     - `METHOD_BUFFERED` (`0`): Input/output via `Irp->AssociatedIrp.SystemBuffer`.
     - `METHOD_IN_DIRECT` / `METHOD_OUT_DIRECT` (`1`/`2`): Uses `MdlAddress`.
     - `METHOD_NEITHER` (`3`): Raw user pointers (`Type3InputBuffer` / `UserBuffer`)—often prone to missing probe/validation checks.

3. **Audit Input Validation:**
   - Trace how user-supplied buffers reach dangerous sinks (`MmMapIoSpace`, `memcpy`, `MmCopyVirtualMemory`, `wrmsr`).
   - Look for missing range checks (e.g., allowing physical addresses outside MMIO ranges, or arbitrary virtual addresses without `ProbeForRead`/`ProbeForWrite`).
   - Record exact input struct layouts (address, size, offset, value) required to trigger each IOCTL.

4. **Automated Symbolic Execution (Optional):**
   - Tools like **angr** or **IOCTLance** can symbolically trace from `IRP_MJ_DEVICE_CONTROL` to target sinks (`MmMapIoSpace`, `ZwOpenProcess`) and automatically synthesize the `IoControlCode` and input buffer constraints.

---

## 4. Why Drivers Still Get Flagged (Anti-Cheat & EDR Telemetry)

Finding an unlisted driver only solves **static hash/certificate blocking**. Kernel-mode anti-cheats (e.g., Easy Anti-Cheat, BattlEye, Vanguard) and modern EDRs monitor multiple runtime signals:

- **Load-Time Telemetry:** `PsSetLoadImageNotifyRoutine` and ETW Threat-Intelligence (`Microsoft-Windows-Threat-Intelligence`) log every driver load, including its path, certificate chain, and registry service entry (`HKLM\SYSTEM\CurrentControlSet\Services`).
- **Known-Bad Behavioral Patterns:**
  - Loading a driver from a temporary directory (`AppData`, `Temp`), unloading it immediately after use, or deleting the service key right after load.
  - Using a known hardware-utility driver when the corresponding hardware or parent vendor software is not present on the system.
- **Post-Load Anomalies:**
  - Unbacked executable memory in kernel space (allocations via `ExAllocatePool` outside any legitimate module in `PsLoadedModuleList`).
  - System threads (`PsCreateSystemThread`) whose start address or call stack points outside signed modules.
  - Hooked dispatch tables, modified kernel callbacks, or anomalous handle permissions.

---

## 5. Verification in an Isolated Lab

Before testing any candidate IOCTL:
- Use an isolated virtual machine (Hyper-V or VMware) with **WinDbg over KDNET** attached.
- Snapshot the VM before loading the driver, as malformed IOCTL inputs or unvalidated physical writes frequently cause bugchecks (BSODs).
- Validate the exact struct size, `IoControlCode`, and return status (`NTSTATUS`) in a controlled user-mode test harness inside the VM.
