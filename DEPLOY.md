# GitHub Pages Deploy Structure

## Pages Root
- `docs/`

## App Entry
- `docs/index.html` : 랜딩 페이지
- `docs/simulator/index.html` : 시뮬레이터 본 페이지
- `docs/simulator/style.css`
- `docs/simulator/app.js`

## GitHub 설정
1. Repository -> Settings -> Pages
2. Source: `Deploy from a branch`
3. Branch: `main` / Folder: `/docs`
4. 저장 후 발급 URL 접속

예시 URL:
- `https://<github-id>.github.io/<repo>/`
- 시뮬레이터 직접 경로: `https://<github-id>.github.io/<repo>/simulator/`
