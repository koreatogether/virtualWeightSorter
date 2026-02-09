# ⚙️ Obsidian 셋업 가이드 - 전기생산 연구용

## 🎯 추천 설정 (5분 안에 끝내기)

### 1단계: 필수 플러그인 설치

Settings > Community Plugins에서 다음 검색 후 설치:

#### 🔍 **검색/연결 강화**
- **Dataview** - 데이터베이스처럼 쿼리하기
  ```dataview
  TABLE 분류, 효율성, 실용성
  FROM "에너지"
  WHERE 효율성 > 3
  SORT 효율성 DESC
  ```

- **Graph Analysis** - 노트 연결 관계 시각화
- **Tag Wrangler** - 태그 관리 자동화

#### ✍️ **작성 편의성**
- **Templater** - 자동 템플릿 삽입
  ```
  /템플릿: 에너지방법
  → 자동으로 템플릿 로드
  ```

- **Quick Add** - 빠른 메모 추가
- **Paste Image Rename** - 이미지 자동 이름 변경

#### 📊 **데이터 시각화**
- **Excalidraw** - 다이어그램/마인드맵 그리기
- **Mermaid** - 플로우차트 자동 생성
  ```mermaid
  graph TD
    A[전기생산] --> B[사무실]
    A --> C[운동]
    A --> D[차량]
  ```

#### 📅 **프로젝트 관리**
- **Calendar** - 연구 일정 추적
- **Periodic Notes** - 일일/주간/월간 정리

#### 🔗 **백링크/네비게이션**
- **Backlink Panel** - 자동 역링크 생성
- **Breadcrumbs** - 폴더 계층 네비게이션

---

### 2단계: 폴더 구조 셋업

Obsidian 홈 폴더에 다음 구조 생성:

```
Energy-Research/
├── 00_Index.md ⭐ (시작점)
├── 01_사무실/
│   ├── 형광등폐열발전.md
│   ├── 컴퓨터냉각열.md
│   ├── 사람체온에너지.md
│   └── 실내공기흐름.md
├── 02_운동기구/
│   ├── 러닝머신발전.md
│   ├── 무게중심이동.md
│   ├── 압전타일.md
│   └── 회전축발전.md
├── 03_차량/
│   ├── 배기열발전.md
│   ├── 브레이크에너지.md
│   ├── 타이어마찰.md
│   ├── 서스펜션운동.md
│   └── 연료전지.md
├── 04_개인활동/
│   ├── 신발압전.md
│   ├── 손목시계.md
│   ├── 압전패브릭.md
│   └── 심장박동.md
├── 05_환경/
│   ├── 태양광.md
│   ├── 온도차열전기.md
│   ├── 음파에너지.md
│   └── 바람에너지.md
├── 📊_분석/
│   ├── 효율성비교.md
│   ├── 비용분석.md
│   └── 환경영향.md
├── 📚_참고자료/
│   └── 참고논문링크.md
└── 🗂️_Templates/
    ├── 에너지방법_기본.md
    ├── 비교분석표.md
    └── 참고자료.md
```

---

### 3단계: 핵심 설정값

**Settings > Editor에서:**
- ✅ Auto pair brackets
- ✅ Auto pair markdown
- ✅ Fold indent
- ✅ Line numbers

**Settings > Files and Links에서:**
- New link format: `[[title]]`
- Embed markdown links: ✅

**Settings > Appearance에서:**
- Theme: "Default" 또는 "Obsidian"
- Font: "Noto Sans CJK" (한글 최적화)

---

## 🚀 효율적인 작업 흐름

### 방법 1: 빠른 입력 (Quick Add)
```
Ctrl/Cmd + Alt + J
→ "새 에너지 방법 추가" 선택
→ 이름 입력 (예: "태양광발전")
→ 자동으로 템플릿 로드
```

### 방법 2: 음성 입력 (선택사항)
스마트폰 음성 메모 → Obsidian Clipper로 자동 저장

### 방법 3: 웹 클리핑
기사/논문 발견 → Obsidian Web Clipper 확장프로그램으로 클립
→ 자동으로 마크다운 변환

---

## 📊 유용한 쿼리 모음

### Dataview로 자동 테이블 생성

**1. 효율성 높은 순서대로 정렬**
```dataview
TABLE 분류, 효율성, 실용성, 난이도
FROM "Energy-Research"
WHERE 효율성
SORT 효율성 DESC
```

**2. 분류별 그룹화**
```dataview
GROUP BY 분류
WHERE 효율성 > 3
```

**3. 비용 대비 효율 비교**
```dataview
TABLE 설치비용, 효율성, 실용성
FROM "03_차량"
WHERE 설치비용 < 10000000
SORT 실용성 DESC
```

**4. 최근 수정 파일**
```dataview
LIST rows.file.link
FROM "Energy-Research"
WHERE file.mtime >= date(today) - dur(7 days)
SORT file.mtime DESC
```

---

## 🎨 아름다운 포맷팅 팁

### 아이콘 활용
```markdown
🔋 전기
🔥 열에너지
💨 풍력
☀️ 태양광
⚡ 운동에너지
🌍 환경
🚗 차량
🏃 운동
💰 경제성
```

### 태그 체계
```markdown
#분류/사무실
#분류/운동기구
#분류/차량
#분류/개인활동
#분류/환경

#에너지타입/열
#에너지타입/운동
#에너지타입/태양
#에너지타입/풍력

#효율성/높음
#효율성/중간
#효율성/낮음

#상태/연구중
#상태/완료
#상태/검증필요
```

### 강조 문법
```markdown
**굵은 글자**: 중요한 정보
*기울임*: 부차 정보
> 인용구: 참고 자료
`코드`: 수식/수치

| 테이블 | 데이터 |
|--------|--------|
```

---

## 🔄 주간 관리 루틴

**매주 월요일**
```markdown
# 주간 에너지 연구 (주차: W-XX)

## 🎯 이번주 목표
- [ ] 차량 에너지 3가지 완성
- [ ] 비용 분석 업데이트
- [ ] 새로운 참고자료 2개 추가

## 📊 진행률
- 사무실: 3/4 ✅
- 운동기구: 2/4 🔄
- 차량: 1/5 ⏳
- 개인활동: 0/4 ⏳
- 환경: 0/4 ⏳

## 📝 작업 로그
- [날짜]: 어떤 작업을 했는지
```

---

## 💡 고급 팁

### 1. CSS 커스터마이징 (선택)
Settings > Appearance > CSS Snippets에서 커스텀 스타일 적용

### 2. 자동 백업
- 폴더를 OneDrive/Google Drive와 동기화
- Git으로 버전 관리 (선택)

### 3. PDF 임포트
- 논문 PDF → Obsidian으로 드래그
- Zotero 연동으로 자동 인용 추가

### 4. 모바일 싱크 (유료)
- Obsidian Sync 구독 시 자동 동기화
- 스마트폰에서도 수정 가능

---

## 📱 모바일 앱 설정 (무료 방법)

1. Obsidian 폴더를 OneDrive에 저장
2. 스마트폰 OneDrive 앱에서 동기화
3. 마크다운 에디터(예: Markor) 앱으로 편집

---

## ✅ 체크리스트

- [ ] Obsidian 설치
- [ ] 필수 플러그인 5개 설치 (Dataview, Graph Analysis, Templater, Quick Add, Calendar)
- [ ] 폴더 구조 생성
- [ ] 템플릿 파일 배치
- [ ] Index.md 파일 추가
- [ ] 첫 번째 에너지 방법 문서 작성 (테스트)
- [ ] 백링크 테스트 (예: [[테스트]])
- [ ] 태그 적용 테스트

---

**더 궁금한 점?**
- Obsidian 공식 문서: https://help.obsidian.md
- 한글 커뮤니티: https://obsidian.md (Help 탭)
