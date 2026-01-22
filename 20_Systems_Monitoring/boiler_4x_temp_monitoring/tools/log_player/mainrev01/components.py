import tkinter as tk
import math

class JogWheel(tk.Canvas):
    """마우스로 잡고 돌릴 수 있는 조그 다이얼 위젯"""
    def __init__(self, master, command=None, size=140, bg_color="#f0f0f0", **kwargs):
        super().__init__(master, width=size, height=size, bg=bg_color, highlightthickness=0, **kwargs)
        self.command = command
        self.size = size
        self.center = size // 2
        self.radius = (size // 2) - 5
        self.angle = 0  # 현재 각도 (-180 ~ 180)
        self.last_angle_mp = 0 # 마우스 포인터 각도
        self.active = False
        
        self.bind("<Button-1>", self.start_drag)
        self.bind("<B1-Motion>", self.do_drag)
        self.bind("<ButtonRelease-1>", self.stop_drag)
        
        self.draw_wheel()
        
    def draw_wheel(self):
        self.delete("all")
        
        # 1. 외곽 원 (그림자 효과)
        self.create_oval(2, 2, self.size-2, self.size-2, fill="#bdc3c7", outline="#95a5a6", width=2)
        
        # 2. 내부 노브
        inset = 12
        self.create_oval(inset, inset, self.size-inset, self.size-inset, fill="#ecf0f1", outline="#bdc3c7")
        
        # 3. 디테일 (미끄럼 방지 홈)
        for i in range(0, 360, 15): # 더 촘촘하게
            rad = math.radians(i + self.angle)
            x1 = self.center + (self.radius - 15) * math.cos(rad)
            y1 = self.center + (self.radius - 15) * math.sin(rad)
            x2 = self.center + (self.radius - 5) * math.cos(rad)
            y2 = self.center + (self.radius - 5) * math.sin(rad)
            self.create_line(x1, y1, x2, y2, fill="#95a5a6", width=2)

        # 4. 지시선 (현재 회전 상태 표시)
        rad_angle = math.radians(self.angle - 90) # -90: 12시 방향 기준
        end_x = self.center + (self.radius - 20) * math.cos(rad_angle)
        end_y = self.center + (self.radius - 20) * math.sin(rad_angle)
        
        # 중심점
        self.create_oval(self.center-6, self.center-6, self.center+6, self.center+6, fill="#7f8c8d")
        # 지시선
        self.create_line(self.center, self.center, end_x, end_y, width=4, fill="#e74c3c", capstyle=tk.ROUND)

    def get_mouse_angle(self, x, y):
        dx = x - self.center
        dy = y - self.center
        return math.degrees(math.atan2(dy, dx))

    def start_drag(self, event):
        self.active = True
        self.last_angle_mp = self.get_mouse_angle(event.x, event.y)

    def do_drag(self, event):
        if not self.active: return
        
        curr_angle_mp = self.get_mouse_angle(event.x, event.y)
        delta = curr_angle_mp - self.last_angle_mp
        
        if delta > 180: delta -= 360
        if delta < -180: delta += 360
        
        self.angle += delta
        self.angle %= 360 
        
        self.last_angle_mp = curr_angle_mp
        self.draw_wheel()
        
        # 각도 변화량과 민감도 조절
        # 1도만 움직여도 변화하도록 민감하게 반응 (시간 간격 연동 위함)
        if abs(delta) > 1 and self.command:
            # 방향만 전달 (1 or -1)
            direction = 1 if delta > 0 else -1
            self.command(direction)
            
    def stop_drag(self, event):
        self.active = False
