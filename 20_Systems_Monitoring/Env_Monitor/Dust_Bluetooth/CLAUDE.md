# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a keypad lock controller project built for Arduino hardware (Uno/R4 Minima) with supporting Python tools for quality assurance and metrics. The project implements a state-machine-based security system with keypad input, servo motor control, LCD display, LED indicators, and buzzer feedback.

## Core Architecture

### Arduino Components (src/arduino/)
- **arduino.ino**: Main entry point with setup() and loop()
- **KeypadLockController**: Main state machine controller with modes (IDLE, ENTER_PASSWORD, ENTER_ADMIN_PASSWORD)
- **Modular Hardware Controllers**: Each hardware component has dedicated controller classes
  - `keypadManager`: Handles 4x4 keypad input using Adafruit_Keypad library
  - `servoController`: Controls servo motor for lock mechanism
  - `lcdPlay`: Manages 16x2 LCD display output
  - `ledController`: Controls status indication LEDs
  - `buzzerController`: Manages audio feedback
  - `potentiometerController`: Handles manual servo adjustment

### Python Toolchain (tools/)
- **Quality Assurance**: Black, Ruff, MyPy, pytest integration
- **Security Scanning**: TruffleHog-based secret detection
- **Arduino Analysis**: Custom static analysis for Arduino code
- **Metrics Collection**: Unified metrics system with JSON/HTML/Markdown export
- **Integrated Reporting**: Combined quality, security, and metrics reports

## Common Development Commands

### Quality Checks and Testing
```bash
# Run all integrated checks (Python quality, Arduino analysis, security scan)
python tools/run_all_checks.py

# Individual quality checks
python tools/quality/quality_check.py          # Python code quality
python tools/quality/arduino_check.py          # Arduino code analysis
python tools/security/trufflehog_check.py      # Security scan

# Auto-fix Python issues
python tools/quality/auto_fix.py
python tools/quality/enhanced_auto_fix.py
```

### Metrics and Reporting
```bash
# Generate comprehensive metrics report
python tools/metrics/run_metrics_simple.py

# Individual metric collection
python tools/metrics/arduino_metrics.py        # Arduino-specific metrics
python tools/metrics/python_coverage.py        # Python test coverage
python tools/analysis/analyze_metrics.py       # Analyze existing reports
```

### Environment Setup
```bash
# Windows environment activation
activate_env.bat      # Batch script
activate_env.ps1      # PowerShell script

# Python environment
uv sync               # Install dependencies via uv
python main.py        # Main Python entry point
python run.py         # Alternative runner
```

## Hardware Testing

### Arduino Development Workflow
The project follows a structured hardware development approach documented in `docs/04_delevelopment/프로젝트순서.md`:

1. **Board Setup**: Prepare Arduino hardware
2. **Keypad Testing**: Verify 4x4 keypad functionality with password scenarios
3. **Servo Integration**: Test servo motor positioning (0-180 degrees)
4. **LCD Display**: Implement status and feedback display
5. **Audio Feedback**: Configure buzzer for success/failure sounds
6. **Potentiometer**: Manual servo control for calibration
7. **LED Indicators**: Status indication (success/failure/admin modes)
8. **Integration Testing**: Full system scenario testing

### Test Scenarios
Test files in `tests/test_50_scenario/` provide hardware validation:
- `keypad_find_r1_c1.ino`: Keypad matrix detection
- `keypad_full_test.ino`: Complete keypad functionality
- `test_50_scenario.ino`: Integrated scenario testing

## Security Configuration

### Password Management
- Default user password: "123456" (hardcoded in KeypadLockController.h:19)
- Default admin password: "000000" (hardcoded in KeypadLockController.h:20)
- Passwords should be moved to secure storage (EEPROM) for production

### Security Features
- State machine prevents unauthorized access attempts
- Admin mode for configuration changes
- Built-in security scanning prevents secret leakage
- Regular dependency vulnerability checks

## Development Guidelines

### Code Organization
- Follow existing modular pattern: each hardware component gets its own controller class
- Use meaningful class names ending in "Controller" or "Manager"
- Maintain state machine pattern in main controller
- Keep hardware abstraction separate from business logic

### Quality Standards
- All Python code must pass Black formatting, Ruff linting, and MyPy type checking
- Arduino code follows custom analysis rules for embedded best practices
- Security scans must pass before commits
- Maintain test coverage for Python components

### Documentation Structure
- Requirements: `docs/01_needs/요구사항.md`
- Scenarios: `docs/02_scenario/` (keypad, servo, integration tests)
- Architecture: `docs/03_architecture/` (design diagrams)
- Development: `docs/04_delevelopment/` (implementation order, advanced features)
- Release: `docs/release/` (version documentation)

## File Structure Context

- `exProject/`: Historical development iterations and reference implementations
- `src/arduino/`: Current production Arduino code
- `src/python/`: Python utilities (currently minimal)
- `tools/`: Complete development toolchain
- `tests/`: Hardware validation scenarios
- `docs/`: Project documentation and planning

## Dependencies

### Python
- pandas, plotly, streamlit for data analysis and visualization
- uv for fast Python package management
- Custom metrics and quality tools

### Arduino
- Adafruit_Keypad library for keypad input
- Servo library for motor control
- Standard Arduino libraries for LCD, tone, and GPIO

Run quality checks before committing any changes to ensure code standards are maintained.