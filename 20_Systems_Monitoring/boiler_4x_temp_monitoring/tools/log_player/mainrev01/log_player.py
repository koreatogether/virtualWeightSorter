import tkinter as tk
from tkinter import filedialog
from config import DEFAULT_LOG_DIR
from app import LogPlayerApp

def main():
    print("Tkinter 초기화 중...")
    try:
        root = tk.Tk()
        print("Tkinter 초기화 완료.")
    except Exception as e:
        print(f"Tkinter 초기화 실패: {e}")
        return

    root.withdraw()
    
    print("폴더 선택 다이얼로그 대기 중...")
    selected_dir = filedialog.askdirectory(
        title="로그 데이터 폴더를 선택하세요",
        initialdir=DEFAULT_LOG_DIR,
        mustexist=True
    )

    if not selected_dir:
        print("폴더 선택이 취소되었습니다.")
        root.destroy()
        return

    print(f"선택된 폴더: {selected_dir}")
    root.deiconify()
    app = LogPlayerApp(root, selected_dir)
    print("메인 루프 시작.")
    root.mainloop()

if __name__ == "__main__":
    main()
