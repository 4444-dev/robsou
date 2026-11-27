# Nexus Internal — Setup & Troubleshooting Guide

If you see **"Impossible d'initialiser le pilote (lancez en Administrateur)"** or **`Failed to open driver handle. Error: 2`**, something on Windows is blocking the driver or mapper from loading. Follow the steps below in order.

---

## 1. Run as Administrator
- Right-click `ioctl-injector.exe` and select **Run as administrator**.
- Do not run it from a compressed `.zip` archive — extract it to a normal folder first (for example `C:\Nexus\`).

---

## 2. Disable Windows Defender & Antivirus
Windows Defender and third-party antivirus software automatically quarantine or block temporary driver/mapper files in `%TEMP%`.

### Windows Security (Windows Defender)
1. Open **Start** and search for **Windows Security**.
2. Go to **Virus & threat protection** -> **Manage settings**.
3. Turn **OFF**:
   - **Real-time protection**
   - **Cloud-delivered protection**
   - **Automatic sample submission**
   - **Tamper Protection** (if you are applying registry/system changes)
4. *(Optional)* Under **Exclusions** -> **Add or remove exclusions**, add:
   - The folder containing `ioctl-injector.exe`
   - Your `%TEMP%` folder (`C:\Users\<YourUser>\AppData\Local\Temp`)

### Third-Party Antivirus
If you have **Malwarebytes, Avast, AVG, Bitdefender, Kaspersky, Norton, ESET, or McAfee** installed:
- Temporarily disable all shields / real-time protection before launching `ioctl-injector.exe`.

---

## 3. Disable Memory Integrity (HVCI) & Vulnerable Driver Blocklist
Windows 11 (and updated Windows 10) blocks unsigned or mapped kernel drivers by default using Core Isolation.

1. Open **Windows Security**.
2. Go to **Device security** -> **Core isolation details**.
3. Turn **OFF**:
   - **Memory integrity**
   - **Microsoft Vulnerable Driver Blocklist** (if visible in the UI)
4. **Restart your PC** for the change to take effect.

> **Note:** If **Microsoft Vulnerable Driver Blocklist** is greyed out or not visible in Windows Security, open **Command Prompt as Administrator** and run:
> ```cmd
> reg add "HKLM\SYSTEM\CurrentControlSet\Control\CI\Config" /v VulnerableDriverBlocklistEnable /t REG_DWORD /d 0 /f
> ```
> Then **restart your PC**.

---

## 4. Close Other Kernel-Level Software
Other background anti-cheats or security tools can block driver initialization:
- **Riot Vanguard** (exit from the system tray and reboot if needed)
- **FACEIT Anti-Cheat**
- **ESEA / BattlEye / Easy Anti-Cheat** left running from another game

---

## 5. Check Network / Firewall / VPN
The loader fetches the driver and mapper from GitHub and verifies your license via KeyAuth:
- Make sure your firewall or VPN is not blocking `github.com`, `raw.githubusercontent.com`, or `keyauth.win`.
- If you see `Failed to download driver or mapper`, disable your VPN/proxy temporarily and try again.

---

## 6. Recommended Launch Order
1. Disable Antivirus / Windows Defender real-time protection.
2. Confirm **Memory integrity** and **Vulnerable Driver Blocklist** are **OFF** (and PC restarted if just changed).
3. Right-click `ioctl-injector.exe` -> **Run as administrator**.
4. Enter your license key.
5. Wait for the popup: **"Lancez ROB RUN, allez dans le lobby puis appuyez sur OK pour lancer."**
6. Launch **ROB RUN**, load into the **lobby**, then click **OK** on the popup.
7. Once in-game, press **`INSERT`** or **`F8`** to open/close the menu.
