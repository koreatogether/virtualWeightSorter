# CHANGELOG — mainRev08_seeed_esp32C3_03

All notable changes for this revision will be documented in this file.

## [Unreleased] - 2026-02-06
### Added
- Created `mainRev08_seeed_esp32C3_03` by copying files from `mainRev08_seeed_esp32C3_02` for stabilization and field testing.
- Added `raptor_checklist.*` for structured verification and `CHANGELOG.md` template.
- Added CI workflow `.github/workflows/pio-build.yml` to validate PlatformIO build.

### To Do
- Implement WiFi auto-reconnect and server restart logic (A1).
- Add Nextion handshake/retry state machine and error indicator (A2).
- Add power consumption test reports and recommended PSU spec (A3).
- Make dashboard auth configurable and secure (A4).