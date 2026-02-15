# Nextion 선 그리기(line) 및 두께 구현 팁

Nextion Basic 모델은 기본적으로 선 굵기 조절 속성이 없습니다. 하지만 `line` 명령어를 응용하여 커스텀 선 그래프를 구현할 수 있습니다.

### 1. 전역 좌표 및 두께 꼼수 (Pseudo-Thickness)
`line x1, y1, x2, y2, color` 명령어를 실행할 때, Y축 좌표에 오프셋을 주어 여러 번 그리면 두꺼운 선 효과를 낼 수 있습니다.

```cpp
// 7픽셀 두께 선 그리기 예시
for (int i = -3; i <= 3; i++) {
    HMISerial.print("line ");
    HMISerial.print(lastX);    HMISerial.print(",");
    HMISerial.print(lastY + i); HMISerial.print(",");
    HMISerial.print(newX);     HMISerial.print(",");
    HMISerial.print(newY + i); HMISerial.print(",2047");
    HMISerial.write(0xFF); HMISerial.write(0xFF); HMISerial.write(0xFF);
}
```

### 2. 주요 도법 명령어
- **영역 채우기**: `fill x, y, w, h, color` (화면 초기화 시 유용)
- **선 그리기**: `line x1, y1, x2, y2, color`
- **텍스트 출력**: `xstr x, y, w, h, fontid, pcolor, bcolor, xcen, ycen, sta, "content"`

### 3. 주의사항
- **통신량**: 선을 두껍게 그릴수록 `line` 명령어를 많이 보내야 하므로 통신 부하가 늘어납니다. 보드레이트가 낮은 경우(9600) 갱신 속도가 느려질 수 있습니다.
- **좌표계**: Nextion은 좌측 상단이 (0,0)입니다. 그래프를 그릴 때 Y축은 `(전체높이 - 값)` 방식으로 반전 계산이 필요합니다.
