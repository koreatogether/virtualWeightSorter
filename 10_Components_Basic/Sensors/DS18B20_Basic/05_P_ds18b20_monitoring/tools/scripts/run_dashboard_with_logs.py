#!/usr/bin/env python3
"""
Run dashboard and ensure Arduino auto-connect (moved to tools/scripts)
"""

from pathlib import Path
import sys

project_root = Path(__file__).parents[2]
sys.path.insert(0, str(project_root))

def _auto_connect(pm):
    """Try to scan and connect to the first available port using PortManager API."""
    ports = pm.scan_ports()
    if not ports:
        return False
    return pm.connect(ports[0])


def main():
    print("🚀 DS18B20 모니터 대시보드 (자동 연결 포함) 시작")

    from src.python.services.port_manager import port_manager

    success = _auto_connect(port_manager)

    if success:
        current_port = port_manager.get_current_port()
        print(f"✅ {current_port} 연결 성공!")
    else:
        print("⚠️ 자동 연결 실패 - 수동 연결 또는 대시보드에서 연결하세요")

    print("\nDash is running on http://127.0.0.1:8050/")
    print("Press CTRL+C to quit")

    try:
        from src.python.app import app
        app.run(host="127.0.0.1", port=8050, debug=False)
    except KeyboardInterrupt:
        print("\n⏹️ 사용자 중단")
    except Exception as e:
        print(f"❌ 대시보드 실행 오류: {e}")
        import traceback
        traceback.print_exc()
    finally:
        try:
            if 'port_manager' in locals() and port_manager:
                port_manager.disconnect()
        except Exception:
            pass
        print("✅ 대시보드 종료 완료")

if __name__ == "__main__":
    main()
