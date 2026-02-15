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
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom"></script>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background-color: #f4f7f6; margin: 0; display: flex; flex-direction: column; align-items: center; }
        header { background-color: #2c3e50; color: white; width: 100%; padding: 1rem 0; text-align: center; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .container { max-width: 800px; width: 95%; margin: 20px auto; display: flex; flex-direction: column; gap: 30px; }
        .controls { background: white; padding: 15px; border-radius: 12px; box-shadow: 0 2px 4px rgba(0,0,0,0.05); display: flex; justify-content: center; gap: 20px; align-items: center; margin-bottom: 10px; }
        .group { display: grid; grid-template-columns: repeat(2, 1fr); gap: 15px; background: rgba(0,0,0,0.02); padding: 15px; border-radius: 15px; border: 1px dashed #ccc; }
        .card { background: white; padding: 20px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); text-align: center; transition: transform 0.2s; }
        .card:hover { transform: translateY(-3px); }
        .card h3 { margin: 0; color: #7f8c8d; font-size: 0.9rem; text-transform: uppercase; }
        .card .value { font-size: 2rem; font-weight: bold; margin: 10px 0; color: #2c3e50; }
        .card .unit { font-size: 1rem; color: #95a5a6; }
        .card.delta { grid-column: span 2; background: #ecf0f1; border-top: 4px solid #3498db; }
        .chart-container { grid-column: span 2; background: white; padding: 15px; border-radius: 12px; height: 300px; position: relative; }
        .status { margin-top: 20px; color: #95a5a6; font-size: 0.8rem; padding-bottom: 20px; }
        .online { color: #27ae60; font-weight: bold; }
        select, button.control-btn { padding: 8px 12px; border-radius: 6px; border: 1px solid #ddd; background: white; cursor: pointer; }
        button.control-btn:hover { background: #f0f0f0; }
        @media (max-width: 400px) {
            .card .value { font-size: 1.5rem; }
            .group { padding: 10px; gap: 10px; }
            .chart-container { height: 200px; }
        }
    </style>
</head>
<body>
    <header>
        <h1>보일러 온도 모니터링</h1>
    </header>
    <div class="container" id="sensor-container">
        <div class="controls">
            <div>
                <label>갱신 주기: </label>
                <select id="update-interval" onchange="changeInterval(this.value)">
                    <optgroup label="실시간 (초)">
                        <option value="1000">1초</option>
                        <option value="2000" selected>2초</option>
                        <option value="5000">5초</option>
                        <option value="10000">10초</option>
                    </optgroup>
                    <optgroup label="모니터링 (분)">
                        <option value="60000">1분</option>
                        <option value="180000">3분</option>
                        <option value="300000">5분</option>
                        <option value="600000">10분</option>
                        <option value="1800000">30분</option>
                    </optgroup>
                    <optgroup label="장기 모니터링 (시간)">
                        <option value="3600000">1시간</option>
                        <option value="10800000">3시간</option>
                        <option value="18000000">5시간</option>
                    </optgroup>
                </select>
            </div>
            <button class="control-btn" onclick="resetZoom()">그래프 줌 초기화</button>
        </div>

        <!-- 그룹 1: S1, S2 및 차이 -->
        <div class="group">
            <div class="card"><h3>S1</h3><div class="value" id="s1">--.-</div><span class="unit">°C</span></div>
            <div class="card"><h3>S2</h3><div class="value" id="s2">--.-</div><span class="unit">°C</span></div>
            <div class="card delta"><h3>DT (S1-S2)</h3><div class="value" id="dt1">--.-</div><span class="unit">°C</span></div>
            <div class="chart-container">
                <canvas id="chart1"></canvas>
            </div>
        </div>

        <!-- 그룹 2: S3, S4 및 차이 -->
        <div class="group">
            <div class="card"><h3>S3</h3><div class="value" id="s3">--.-</div><span class="unit">°C</span></div>
            <div class="card"><h3>S4</h3><div class="value" id="s4">--.-</div><span class="unit">°C</span></div>
            <div class="card delta"><h3>DT (S3-S4)</h3><div class="value" id="dt2">--.-</div><span class="unit">°C</span></div>
            <div class="chart-container">
                <canvas id="chart2"></canvas>
            </div>
        </div>
    <div class="status">
        상태: <span id="connection-status" class="online">연결됨</span> | 
        기록 상태: <span id="logging-status">--</span> | 
        마지막 갱신: <span id="last-update">--:--:--</span>
    </div>

    <script>
        let chart1, chart2;
        let updateTimer;
        const maxDataPoints = 50;

        function initCharts() {
            try {
                const commonOptions = {
                    responsive: true,
                    maintainAspectRatio: false,
                    scales: {
                        x: { display: true, title: { display: true, text: '시간' } },
                        y: { display: true, title: { display: true, text: '온도 (°C)' }, suggestMin: 20, suggestMax: 80 }
                    },
                    plugins: {
                        zoom: {
                            zoom: { wheel: { enabled: true }, pinch: { enabled: true }, mode: 'xy' },
                            pan: { enabled: true, mode: 'xy' }
                        }
                    },
                    animation: { duration: 400 }
                };

                const ctx1 = document.getElementById('chart1');
                const ctx2 = document.getElementById('chart2');

                if (ctx1 && typeof Chart !== 'undefined') {
                    chart1 = new Chart(ctx1, {
                        type: 'line',
                        data: { labels: [], datasets: [
                            { label: 'S1', borderColor: '#e74c3c', data: [], tension: 0.3, fill: false },
                            { label: 'S2', borderColor: '#3498db', data: [], tension: 0.3, fill: false }
                        ]},
                        options: commonOptions
                    });
                }

                if (ctx2 && typeof Chart !== 'undefined') {
                    chart2 = new Chart(ctx2, {
                        type: 'line',
                        data: { labels: [], datasets: [
                            { label: 'S3', borderColor: '#2ecc71', data: [], tension: 0.3, fill: false },
                            { label: 'S4', borderColor: '#f1c40f', data: [], tension: 0.3, fill: false }
                        ]},
                        options: commonOptions
                    });
                }
            } catch (e) {
                console.error("Chart initialization failed:", e);
            }
        }

        function updateData() {
            fetch('/data')
                .then(response => {
                    if (!response.ok) throw new Error('Network response was not ok');
                    return response.json();
                })
                .then(data => {
                    const now = new Date().toLocaleTimeString();
                    
                    // UI Update (텍스트 값 우선 업데이트)
                    if (data.s1 !== undefined) document.getElementById('s1').innerText = data.s1.toFixed(1);
                    if (data.s2 !== undefined) document.getElementById('s2').innerText = data.s2.toFixed(1);
                    if (data.s3 !== undefined) document.getElementById('s3').innerText = data.s3.toFixed(1);
                    if (data.s4 !== undefined) document.getElementById('s4').innerText = data.s4.toFixed(1);
                    
                    if (data.s1 !== undefined && data.s2 !== undefined) {
                        document.getElementById('dt1').innerText = (Math.abs(data.s1 - data.s2)).toFixed(1);
                    }
                    if (data.s3 !== undefined && data.s4 !== undefined) {
                        document.getElementById('dt2').innerText = (Math.abs(data.s3 - data.s4)).toFixed(1);
                    }

                    // 기록 상태 업데이트
                    const logStatus = document.getElementById('logging-status');
                    if (data.logging) {
                        logStatus.innerText = "기록 중 (PC 연동)";
                        logStatus.style.color = "#27ae60";
                        logStatus.style.fontWeight = "bold";
                    } else {
                        logStatus.innerText = "중단됨 (PC 미연결)";
                        logStatus.style.color = "#e74c3c";
                        logStatus.style.fontWeight = "normal";
                    }

                    document.getElementById('last-update').innerText = now;
                    document.getElementById('connection-status').innerText = "연결됨";
                    document.getElementById('connection-status').className = "online";

                    // Chart Update (차트가 존재할 때만 업데이트)
                    if (chart1) {
                        if (chart1.data.labels.length > maxDataPoints) {
                            chart1.data.labels.shift();
                            chart1.data.datasets.forEach(dataset => dataset.data.shift());
                        }
                        chart1.data.labels.push(now);
                        chart1.data.datasets[0].data.push(data.s1);
                        chart1.data.datasets[1].data.push(data.s2);
                        chart1.update('none');
                    }

                    if (chart2) {
                        if (chart2.data.labels.length > maxDataPoints) {
                            chart2.data.labels.shift();
                            chart2.data.datasets.forEach(dataset => dataset.data.shift());
                        }
                        chart2.data.labels.push(now);
                        chart2.data.datasets[0].data.push(data.s3);
                        chart2.data.datasets[1].data.push(data.s4);
                        chart2.update('none');
                    }
                })
                .catch(err => {
                    document.getElementById('connection-status').innerText = "연동 에러";
                    document.getElementById('connection-status').className = "";
                    console.error('Fetch error:', err);
                });
        }

        function changeInterval(ms) {
            clearInterval(updateTimer);
            updateTimer = setInterval(updateData, ms);
        }

        function resetZoom() {
            chart1.resetZoom();
            chart2.resetZoom();
        }

        window.onload = () => {
            initCharts();
            updateData();
            updateTimer = setInterval(updateData, 2000);
        };
    </script>
</body>
</html>
)=====";

#endif
