#ifndef DASHBOARD_H
#define DASHBOARD_H

const char LOGIN_HTML[] = R"=====(
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>로그인 - Boiler Monitor</title>
    <style>
        body { font-family: sans-serif; background: #2c3e50; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .login-box { background: white; padding: 40px; border-radius: 10px; box-shadow: 0 10px 25px rgba(0,0,0,0.2); text-align: center; }
        input { width: 100%; padding: 10px; margin: 10px 0; border: 1px solid #ddd; border-radius: 5px; box-sizing: border-box; }
        button { width: 100%; padding: 10px; background: #3498db; color: white; border: none; border-radius: 5px; cursor: pointer; }
        button:hover { background: #2980b9; }
        .error { color: #e74c3c; font-size: 0.8rem; margin-top: 10px; }
    </style>
</head>
<body>
    <div class="login-box">
        <h2>환경 모니터링 시스템</h2>
        <form method="GET" action="/login">
            <input type="password" name="pass" placeholder="비밀번호를 입력하세요" required>
            <button type="submit">로그인</button>
        </form>
    </div>
</body>
</html>
)=====";

const char INDEX_HTML[] = R"=====(
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Boiler Temp Monitor</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background-color: #f4f7f6; margin: 0; display: flex; flex-direction: column; align-items: center; }
        header { background-color: #2c3e50; color: white; width: 100%; padding: 1rem 0; text-align: center; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .container { max-width: 800px; width: 95%; margin: 20px auto; display: flex; flex-direction: column; gap: 30px; }
        .group { display: grid; grid-template-columns: repeat(2, 1fr); gap: 15px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 15px; border: 1px dashed #ccc; }
        .card { background: white; padding: 20px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); text-align: center; transition: transform 0.2s; }
        .card:hover { transform: translateY(-3px); }
        .card h3 { margin: 0; color: #7f8c8d; font-size: 0.9rem; text-transform: uppercase; }
        .card .value { font-size: 2rem; font-weight: bold; margin: 10px 0; color: #2c3e50; }
        .card .unit { font-size: 1rem; color: #95a5a6; }
        .card.delta { grid-column: span 2; background: #ecf0f1; border-top: 4px solid #3498db; }
        .status { margin-top: 20px; color: #95a5a6; font-size: 0.8rem; padding-bottom: 20px; }
        .online { color: #27ae60; font-weight: bold; }
        @media (max-width: 400px) {
            .card .value { font-size: 1.5rem; }
            .group { padding: 10px; gap: 10px; }
        }
    </style>
</head>
<body>
    <header>
        <h1>보일러 온도 모니터링</h1>
    </header>
    <div class="container" id="sensor-container">
        <!-- 그룹 1: S1, S2 및 차이 -->
        <div class="group">
            <div class="card"><h3>S1</h3><div class="value" id="s1">--.-</div><span class="unit">°C</span></div>
            <div class="card"><h3>S2</h3><div class="value" id="s2">--.-</div><span class="unit">°C</span></div>
            <div class="card delta"><h3>DT (S1-S2)</h3><div class="value" id="dt1">--.-</div><span class="unit">°C</span></div>
        </div>

        <!-- 그룹 2: S3, S4 및 차이 -->
        <div class="group">
            <div class="card"><h3>S3</h3><div class="value" id="s3">--.-</div><span class="unit">°C</span></div>
            <div class="card"><h3>S4</h3><div class="value" id="s4">--.-</div><span class="unit">°C</span></div>
            <div class="card delta"><h3>DT (S3-S4)</h3><div class="value" id="dt2">--.-</div><span class="unit">°C</span></div>
        </div>
    </div>
    <div class="status">상태: <span id="connection-status" class="online">연결됨</span> | 마지막 갱신: <span id="last-update">--:--:--</span></div>

    <script>
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('s1').innerText = data.s1.toFixed(1);
                    document.getElementById('s2').innerText = data.s2.toFixed(1);
                    document.getElementById('s3').innerText = data.s3.toFixed(1);
                    document.getElementById('s4').innerText = data.s4.toFixed(1);
                    document.getElementById('dt1').innerText = (Math.abs(data.s1 - data.s2)).toFixed(1);
                    document.getElementById('dt2').innerText = (Math.abs(data.s3 - data.s4)).toFixed(1);
                    document.getElementById('last-update').innerText = new Date().toLocaleTimeString();
                    document.getElementById('connection-status').innerText = "연결됨";
                    document.getElementById('connection-status').className = "online";
                })
                .catch(err => {
                    document.getElementById('connection-status').innerText = "재연결 중...";
                    document.getElementById('connection-status').className = "";
                    console.error('Fetch error:', err);
                });
        }
        setInterval(updateData, 2000);
        window.onload = updateData;
    </script>
</body>
</html>
)=====";

#endif
