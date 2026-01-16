#!/usr/bin/env python3
"""Small firmware probe script for exercising new commands and tracking config_dirty transitions.

Usage: run while device is connected to COM port. Adjust COM port as needed.
"""

import json
import time
from pathlib import Path

import serial

PORT = "COM4"
BAUD = 115200

LOG = Path("./tools/firmware_probe/probe_log.txt")


def send_command(ser, cmd: dict):
    msg = json.dumps(cmd, ensure_ascii=False) + "\n"
    ser.write(msg.encode("utf-8"))
    ser.flush()


def read_response(ser, timeout=1.0):
    buf = ""
    end = time.time() + timeout
    while time.time() < end:
        if ser.in_waiting:
            data = ser.read(ser.in_waiting).decode("utf-8", errors="ignore")
            buf += data
            if "\n" in buf:
                line, buf = buf.split("\n", 1)
                try:
                    return json.loads(line)
                except Exception:
                    return {"raw": line}
        time.sleep(0.01)
    return None


def probe(port=PORT):
    with serial.Serial(port, BAUD, timeout=0.1) as ser:
        print("Connected to", port)
        # ensure log directory exists
        LOG.parent.mkdir(parents=True, exist_ok=True)
        # get initial status
        send_command(ser, {"type": "command", "command": "get_status"})
        resp = read_response(ser, timeout=1.5)
        print("status:", resp)
        with LOG.open("a", encoding="utf-8") as fh:
            fh.write(f"{time.asctime()} STATUS {resp}\n")

        # list sensors
        send_command(ser, {"type": "command", "command": "list_sensors"})
        resp = read_response(ser, timeout=1.0)
        print("list_sensors:", resp)

        # change debounce briefly and observe dirty
        send_command(
            ser,
            {
                "type": "command",
                "command": "set_config",
                "config_type": "debounce_ms",
                "new_value": 1200,
            },
        )
        resp = read_response(ser, timeout=1.0)
        print("set debounce resp:", resp)

        # poll status until dirty clears
        start = time.time()
        last_dirty = None
        while time.time() - start < 5:
            send_command(ser, {"type": "command", "command": "get_status"})
            resp = read_response(ser, timeout=0.5)
            print("polled:", resp)
            if resp and isinstance(resp, dict) and "config_dirty" in resp:
                if last_dirty is None:
                    last_dirty = resp["config_dirty"]
                if last_dirty and not resp["config_dirty"]:
                    print("dirty cleared")
                    break
            time.sleep(0.4)

        # test set_sensor_data
        send_command(
            ser, {"type": "command", "command": "set_sensor_data", "new_value": 2}
        )
        resp = read_response(ser, timeout=1.0)
        print("set_sensor_data resp:", resp)

        # final status
        send_command(ser, {"type": "command", "command": "get_status"})
        resp = read_response(ser, timeout=1.0)
        print("final status:", resp)


if __name__ == "__main__":
    try:
        probe()
    except Exception as exc:
        print("Probe failed:", exc)
