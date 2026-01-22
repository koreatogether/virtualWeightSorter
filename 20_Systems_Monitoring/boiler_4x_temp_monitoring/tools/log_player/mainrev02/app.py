import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
import os
import json
import glob
from datetime import datetime, timedelta
import matplotlib.dates as mdates
import matplotlib.transforms as transforms
import bisect

from config import WINDOW_TITLE, VIEW_INTERVALS
from components import JogWheel

class LogPlayerApp:
    def __init__(self, root, log_dir):
        self.root = root
        self.log_dir = log_dir
        self.root.title(f"{WINDOW_TITLE} - {self.log_dir}")
        self.root.geometry("1600x900")

        # 데이터 저장소
        self.times = []
        self.s1_data, self.s2_data, self.diff1_data = [], [], []
        self.s3_data, self.s4_data, self.diff2_data = [], [], []

        # 상태 변수 (그룹별 독립 제어)
        self.current_idx1 = 0
        self.current_idx2 = 0
        self.view_range_sec = 60 # 기본 1분
        
        # 드래그/패닝 상태 변수
        self.is_panning = False
        self.pan_start_x = None
        self.pan_start_xlim1 = None
        self.pan_start_xlim2 = None

        # 임계치(Threshold) 설정
        self.upper_limit = None # 계획상한온도
        self.lower_limit = None # 계획하한온도
        
        self.cursor_line1 = None
        self.cursor_line2 = None
        self.cursor_text1 = None
        self.cursor_text2 = None

        self._init_ui()
        self.load_data()

    def _init_ui(self):
        # 전체 레이아웃 구성을 위해 PanedWindow 사용 (좌: 컨트롤, 우: 그래프)
        main_pane = tk.Frame(self.root)
        main_pane.pack(fill=tk.BOTH, expand=True)

        # === 좌측 컨트롤 패널 ===
        left_panel = tk.Frame(main_pane, width=350, bg="#ecf0f1")
        left_panel.pack(side=tk.LEFT, fill=tk.Y)
        left_panel.pack_propagate(False) # 크기 고정

        # 1. 파일 정보
        info_frame = ttk.LabelFrame(left_panel, text="데이터 정보", padding="10")
        info_frame.pack(fill=tk.X, padx=10, pady=10)
        self.lbl_file_count = ttk.Label(info_frame, text="파일: 0개")
        self.lbl_file_count.pack(anchor=tk.W)
        self.lbl_data_count = ttk.Label(info_frame, text="데이터: 0건")
        self.lbl_data_count.pack(anchor=tk.W)

        # 2. 공통 설정 (뷰 범위)
        setting_frame = ttk.LabelFrame(left_panel, text="뷰 범위 (시간 간격)", padding="10")
        setting_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.interval_var = tk.StringVar(value="5시간")
        self.interval_combo = ttk.Combobox(setting_frame, textvariable=self.interval_var, 
                                         values=list(VIEW_INTERVALS.keys()), state="readonly")
        self.interval_combo.pack(fill=tk.X, pady=5)
        self.interval_combo.bind("<<ComboboxSelected>>", self.on_interval_change)
        # 초기값 설정
        self.view_range_sec = VIEW_INTERVALS["5시간"]

        # 3. 그룹 1 컨트롤 (S1-S2)
        g1_frame = ttk.LabelFrame(left_panel, text="그룹 1 (S1-S2) 제어", padding="10")
        g1_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.lbl_time1 = ttk.Label(g1_frame, text="--:--:--", font=("Consolas", 14, "bold"))
        self.lbl_time1.pack(pady=5)
        
        # Jog 1
        jog1_container = tk.Frame(g1_frame)
        jog1_container.pack()
        self.jog1 = JogWheel(jog1_container, command=self.on_jog1_rotate, size=120)
        self.jog1.pack()
        
        # 4. 그룹 2 컨트롤 (S3-S4)
        g2_frame = ttk.LabelFrame(left_panel, text="그룹 2 (S3-S4) 제어", padding="10")
        g2_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.lbl_time2 = ttk.Label(g2_frame, text="--:--:--", font=("Consolas", 14, "bold"))
        self.lbl_time2.pack(pady=5)
        
        # Jog 2
        jog2_container = tk.Frame(g2_frame)
        jog2_container.pack()
        self.jog2 = JogWheel(jog2_container, command=self.on_jog2_rotate, size=120)
        self.jog2.pack()

        # 5. 통계 정보 (Range Stats)
        stats_frame = ttk.LabelFrame(left_panel, text="구간 통계 (현재 뷰)", padding="10")
        stats_frame.pack(fill=tk.X, padx=10, pady=10)
        self.lbl_stats1 = ttk.Label(stats_frame, text="Group 1: -")
        self.lbl_stats1.pack(anchor=tk.W)
        self.lbl_stats2 = ttk.Label(stats_frame, text="Group 2: -")
        self.lbl_stats2.pack(anchor=tk.W)

        # 6. 임계치 설정 (Threshold)
        limit_frame = ttk.LabelFrame(left_panel, text="임계치 설정 (계획온도)", padding="5")
        limit_frame.pack(fill=tk.X, padx=10, pady=10)

        # 상한
        row_upper = tk.Frame(limit_frame)
        row_upper.pack(fill=tk.X, pady=2)
        ttk.Label(row_upper, text="상한(Red):").pack(side=tk.LEFT)
        self.entry_upper = ttk.Entry(row_upper, width=8)
        self.entry_upper.pack(side=tk.RIGHT)
        self.entry_upper.bind("<Return>", lambda e: self.apply_limits())

        # 하한
        row_lower = tk.Frame(limit_frame)
        row_lower.pack(fill=tk.X, pady=2)
        ttk.Label(row_lower, text="하한(Blue):").pack(side=tk.LEFT)
        self.entry_lower = ttk.Entry(row_lower, width=8)
        self.entry_lower.pack(side=tk.RIGHT)
        self.entry_lower.bind("<Return>", lambda e: self.apply_limits())

        # 적용 버튼
        self.btn_limit = ttk.Button(limit_frame, text="적용", command=self.apply_limits)
        self.btn_limit.pack(fill=tk.X, pady=5)

        # === 우측 그래프 영역 ===
        right_panel = tk.Frame(main_pane, bg="white")
        right_panel.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(10, 8))
        # sharex=False로 설정하여 X축 독립적으로 운용
        plt.subplots_adjust(left=0.08, right=0.95, top=0.91, bottom=0.08, hspace=0.3)

        self.canvas = FigureCanvasTkAgg(self.fig, master=right_panel)
        self.canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        # 툴바 추가 (줌/이동 기능)
        self.toolbar = NavigationToolbar2Tk(self.canvas, right_panel)
        self.toolbar.update()
        
        # 줌/팬 이벤트 연결 - 범위 변경 시 통계 업데이트
        self.ax1.callbacks.connect('xlim_changed', self.on_xlim_changed)
        self.ax2.callbacks.connect('xlim_changed', self.on_xlim_changed)

        # 마우스 이벤트 연결 (커서 기능 + 드래그 패닝)
        self.canvas.mpl_connect("motion_notify_event", self.on_mouse_move)
        self.canvas.mpl_connect("button_press_event", self.on_mouse_press)
        self.canvas.mpl_connect("button_release_event", self.on_mouse_release)

    def on_xlim_changed(self, event_ax):
        # 뷰 범위가 변경될 때 통계 업데이트
        if not self.is_panning: 
             self.update_stats()

    def apply_limits(self):
        try:
            u_val = self.entry_upper.get().strip()
            l_val = self.entry_lower.get().strip()
            

            print(f"임계치 적용: 상한={self.upper_limit}, 하한={self.lower_limit}")
            self.upper_limit = float(u_val) if u_val else None
            self.lower_limit = float(l_val) if l_val else None
            
            self.update_plot1()
            self.update_plot2()
        except ValueError:
            messagebox.showerror("입력 오류", "숫자를 입력해주세요.")

    def on_mouse_press(self, event):
        if event.button == 1 and event.inaxes: # Left click
            self.is_panning = True
            self.pan_start_x = event.x
            self.pan_start_xlim1 = self.ax1.get_xlim()
            self.pan_start_xlim2 = self.ax2.get_xlim()
            self.canvas.get_tk_widget().config(cursor="fleur") # Change cursor icon

    def on_mouse_release(self, event):
        if self.is_panning:
            self.is_panning = False
            self.canvas.get_tk_widget().config(cursor="") # Restore cursor
            self.update_stats()

    def update_stats(self):
        if not self.times: return
        
        # Group 1 stats
        x1_min, x1_max = self.ax1.get_xlim()
        
        try:
            t_start = mdates.num2date(x1_min).replace(tzinfo=None)
            t_end = mdates.num2date(x1_max).replace(tzinfo=None)
            
            idx_start = bisect.bisect_left(self.times, t_start)
            idx_end = bisect.bisect_right(self.times, t_end)
            
            if idx_start < idx_end:
                 subset = self.diff1_data[idx_start:idx_end]
                 if subset:
                    avg_val = sum(subset) / len(subset)
                    max_val = max(subset)
                    min_val = min(subset)
                    self.lbl_stats1.config(text=f"G1 ΔT: Max {max_val:.1f} / Avg {avg_val:.1f}")
            else:
                self.lbl_stats1.config(text="G1 ΔT: (범위 내 데이터 없음)")
                
            # Group 2 stats (using ax2 limit)
            x2_min, x2_max = self.ax2.get_xlim()
            t_start2 = mdates.num2date(x2_min).replace(tzinfo=None)
            t_end2 = mdates.num2date(x2_max).replace(tzinfo=None)
            
            idx_start2 = bisect.bisect_left(self.times, t_start2)
            idx_end2 = bisect.bisect_right(self.times, t_end2)
            
            if idx_start2 < idx_end2:
                 subset2 = self.diff2_data[idx_start2:idx_end2]
                 if subset2:
                    avg_val2 = sum(subset2) / len(subset2)
                    max_val2 = max(subset2)
                    min_val2 = min(subset2)
                    self.lbl_stats2.config(text=f"G2 ΔT: Max {max_val2:.1f} / Avg {avg_val2:.1f}")
            else:
                self.lbl_stats2.config(text="G2 ΔT: (범위 내 데이터 없음)")

        except Exception as e:
            print(f"Stats error: {e}")

    def load_data(self):
        print(f"데이터 로딩 시작: {self.log_dir}")
        search_pattern = os.path.join(self.log_dir, "**", "*.json")
        files = glob.glob(search_pattern, recursive=True)
        
        if not files:
            print("로그 파일을 찾을 수 없습니다.")
            messagebox.showerror("오류", f"로그 파일이 없습니다.\n{self.log_dir}")
            return

        data_list = []
        file_count = len(files)
        self.lbl_file_count.config(text=f"파일: {file_count}개")
        self.root.update()

        print(f"총 {file_count}개 파일 분석 중...")
        for i, fpath in enumerate(files):
            if i % 100 == 0:
                print(f"진행 중... ({i}/{file_count})")
                self.lbl_data_count.config(text=f"로드 중... ({i}/{file_count})")
                self.root.update()
            
            try:
                fname = os.path.basename(fpath)
                if not fname.endswith(".json") or len(fname) < 19:
                    continue
                
                time_str = fname.replace(".json", "")
                dt = datetime.strptime(time_str, "%Y-%m-%d_%H-%M-%S")
                with open(fpath, 'r', encoding='utf-8') as f:
                    content = json.load(f)
                    if 's1' in content:
                        entry = {
                            'dt': dt,
                            's1': content.get('s1', 0), 's2': content.get('s2', 0),
                            's3': content.get('s3', 0), 's4': content.get('s4', 0)
                        }
                        data_list.append(entry)
            except Exception as e:
                pass

        print("데이터 정렬 중...")
        data_list.sort(key=lambda x: x['dt'])
        
        if not data_list:
            print("유효한 데이터가 없습니다.")
            messagebox.showwarning("데이터 없음", "유효한 데이터가 없습니다.")
            return

        self.times = [x['dt'] for x in data_list]
        self.s1_data = [x['s1'] for x in data_list]
        self.s2_data = [x['s2'] for x in data_list]
        self.s3_data = [x['s3'] for x in data_list]
        self.s4_data = [x['s4'] for x in data_list]
        self.diff1_data = [abs(x['s1'] - x['s2']) for x in data_list]
        self.diff2_data = [abs(x['s3'] - x['s4']) for x in data_list]

        self.lbl_data_count.config(text=f"데이터: {len(data_list):,}건")
        print(f"데이터 로딩 완료: {len(data_list)}건")
        
        # 마지막 시점으로 초기화
        self.current_idx1 = len(self.times) - 1
        self.current_idx2 = len(self.times) - 1
        
        self.update_plot1()
        self.update_plot2()

    def get_time_step(self):
        return self.view_range_sec * 0.1

    def find_nearest_index(self, target_time):
        idx = bisect.bisect_left(self.times, target_time)
        if idx >= len(self.times):
            idx = len(self.times) - 1
        return idx
        
    def move_time(self, group, direction):
        step_seconds = self.view_range_sec * 0.1 
        
        if group == 1:
            curr_time = self.times[self.current_idx1]
            new_time = curr_time + timedelta(seconds=step_seconds * direction)
            self.current_idx1 = self.find_nearest_index(new_time)
            self.update_plot1()
            
        elif group == 2:
            curr_time = self.times[self.current_idx2]
            new_time = curr_time + timedelta(seconds=step_seconds * direction)
            self.current_idx2 = self.find_nearest_index(new_time)
            self.update_plot2()

    def on_jog1_rotate(self, direction):
        self.move_time(1, direction)

    def on_jog2_rotate(self, direction):
        self.move_time(2, direction)

    def on_interval_change(self, event):
        val = self.interval_var.get()
        self.view_range_sec = VIEW_INTERVALS.get(val, 60)
        self.update_plot1()
        self.update_plot2()

    def update_plot1(self):
        if not self.times: return
        
        idx = self.current_idx1
        curr_time = self.times[idx]
        self.lbl_time1.config(text=curr_time.strftime("%Y-%m-%d\n%H:%M:%S"))

        start_time = curr_time - timedelta(seconds=self.view_range_sec)
        end_time = curr_time + timedelta(seconds=self.view_range_sec * 0.1)

        self.ax1.clear()
        self.ax1.plot(self.times, self.s1_data, label='S1', color='#e74c3c', lw=1.5)
        self.ax1.plot(self.times, self.s2_data, label='S2', color='#3498db', lw=1.5)
        self.ax1.fill_between(self.times, self.s1_data, self.s2_data, color='gray', alpha=0.1)
        self.ax1.plot(self.times, self.diff1_data, label='dT', color='#8e44ad', ls='--', alpha=0.8)
        
        # 임계치 라인 그리기
        if self.upper_limit is not None:
             self.ax1.axhline(y=self.upper_limit, color='red', linestyle='--', alpha=0.9, linewidth=1.5, label='Max Limit', zorder=5)
        if self.lower_limit is not None:
             self.ax1.axhline(y=self.lower_limit, color='blue', linestyle='--', alpha=0.9, linewidth=1.5, label='Min Limit', zorder=5)

        self.ax1.set_title(f"Group 1 (S1-S2) Temp Δ: {self.diff1_data[idx]:.1f}°C")
        self.ax1.grid(True, linestyle=':', alpha=0.6)
        self.ax1.legend(loc='upper right', fontsize='small')
        self.ax1.axvline(x=curr_time, color='black', alpha=0.8)

        self.ax1.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
        self.ax1.tick_params(axis='x', rotation=0) 
        
        self.ax1.set_xlim(start_time, end_time)
        self.ax1.set_ylim(0, 90) 

        self.canvas.draw()
        if not self.is_panning:
            self.update_stats()

    def update_plot2(self):
        if not self.times: return

        idx = self.current_idx2
        curr_time = self.times[idx]
        self.lbl_time2.config(text=curr_time.strftime("%Y-%m-%d\n%H:%M:%S"))

        start_time = curr_time - timedelta(seconds=self.view_range_sec)
        end_time = curr_time + timedelta(seconds=self.view_range_sec * 0.1)

        self.ax2.clear()
        self.ax2.plot(self.times, self.s3_data, label='S3', color='#2ecc71', lw=1.5)
        self.ax2.plot(self.times, self.s4_data, label='S4', color='#f1c40f', lw=1.5)
        self.ax2.fill_between(self.times, self.s3_data, self.s4_data, color='gray', alpha=0.1)
        self.ax2.plot(self.times, self.diff2_data, label='dT', color='#e67e22', ls='--', alpha=0.8)

        # 임계치 라인 그리기
        if self.upper_limit is not None:
             self.ax2.axhline(y=self.upper_limit, color='red', linestyle='--', alpha=0.9, linewidth=1.5, label='Max Limit', zorder=5)
        if self.lower_limit is not None:
             self.ax2.axhline(y=self.lower_limit, color='blue', linestyle='--', alpha=0.9, linewidth=1.5, label='Min Limit', zorder=5)
        
        self.ax2.set_title(f"Group 2 (S3-S4) Temp Δ: {self.diff2_data[idx]:.1f}°C")
        self.ax2.grid(True, linestyle=':', alpha=0.6)
        self.ax2.legend(loc='upper right', fontsize='small')
        self.ax2.axvline(x=curr_time, color='black', alpha=0.8)

        self.ax2.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
        self.ax2.tick_params(axis='x', rotation=0)

        self.ax2.set_xlim(start_time, end_time)
        self.ax2.set_ylim(0, 90)

        self.canvas.draw()
        if not self.is_panning:
             self.update_stats()

    def on_mouse_move(self, event):
        # 1. 패닝 처리
        if self.is_panning and event.x is not None:
            dx_pixels = event.x - self.pan_start_x
            
            bbox1 = self.ax1.get_window_extent().transformed(self.fig.dpi_scale_trans.inverted())
            width1 = bbox1.width * self.fig.dpi
            
            x_range1 = self.pan_start_xlim1[1] - self.pan_start_xlim1[0]
            dt_per_pixel = x_range1 / width1 if width1 > 0 else 0
            
            shift = dx_pixels * dt_per_pixel
            
            new_xlim1_0 = self.pan_start_xlim1[0] - shift
            new_xlim1_1 = self.pan_start_xlim1[1] - shift
            
            self.ax1.set_xlim(new_xlim1_0, new_xlim1_1)
            self.ax2.set_xlim(new_xlim1_0, new_xlim1_1)
            
            self.canvas.draw_idle()
            return
            
        # 2. 커서 처리
        if not self.is_panning and event.inaxes and self.times:
            try:
                target_dt = mdates.num2date(event.xdata)
                if target_dt.tzinfo:
                    target_dt = target_dt.replace(tzinfo=None)
            except ValueError:
                return

            idx = bisect.bisect_left(self.times, target_dt)
            if idx >= len(self.times): idx = len(self.times) - 1
            
            curr_time = self.times[idx]
            val_s1 = self.s1_data[idx]
            val_s2 = self.s2_data[idx]
            val_diff1 = self.diff1_data[idx]
            
            val_s3 = self.s3_data[idx]
            val_s4 = self.s4_data[idx]
            val_diff2 = self.diff2_data[idx]

            self.draw_cursor(curr_time, val_s1, val_s2, val_diff1, val_s3, val_s4, val_diff2)

    def draw_cursor(self, time_val, s1, s2, d1, s3, s4, d2):
        if self.cursor_line1 and self.cursor_line1 in self.ax1.lines:
            self.cursor_line1.set_xdata([time_val, time_val])
        else:
            self.cursor_line1 = self.ax1.axvline(x=time_val, color='gray', linestyle='--', alpha=0.8)
        
        text1 = f"[{time_val.strftime('%H:%M:%S')}]\nS1: {s1}°C\nS2: {s2}°C\nΔT: {d1:.1f}°C"
        
        if self.cursor_text1 and self.cursor_text1 in self.ax1.texts:
            self.cursor_text1.set_text(text1)
            self.cursor_text1.set_x(time_val)
        else:
             props = dict(boxstyle='round', facecolor='white', alpha=0.9, edgecolor='gray')
             trans = transforms.blended_transform_factory(self.ax1.transData, self.ax1.transAxes)
             self.cursor_text1 = self.ax1.text(time_val, 0.95, text1, transform=trans,
                                               verticalalignment='top', horizontalalignment='left',
                                               bbox=props, fontsize=9, zorder=10)

        if self.cursor_line2 and self.cursor_line2 in self.ax2.lines:
            self.cursor_line2.set_xdata([time_val, time_val])
        else:
            self.cursor_line2 = self.ax2.axvline(x=time_val, color='gray', linestyle='--', alpha=0.8)
            
        text2 = f"[{time_val.strftime('%H:%M:%S')}]\nS3: {s3}°C\nS4: {s4}°C\nΔT: {d2:.1f}°C"
        
        if self.cursor_text2 and self.cursor_text2 in self.ax2.texts:
            self.cursor_text2.set_text(text2)
            self.cursor_text2.set_x(time_val)
        else:
             props = dict(boxstyle='round', facecolor='white', alpha=0.9, edgecolor='gray')
             trans = transforms.blended_transform_factory(self.ax2.transData, self.ax2.transAxes)
             self.cursor_text2 = self.ax2.text(time_val, 0.95, text2, transform=trans,
                                               verticalalignment='top', horizontalalignment='left',
                                               bbox=props, fontsize=9, zorder=10)

        self.canvas.draw_idle()
