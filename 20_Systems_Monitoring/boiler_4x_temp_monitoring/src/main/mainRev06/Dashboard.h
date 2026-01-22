#ifndef DASHBOARD_H
#define DASHBOARD_H

const char LOGIN_HTML[] = R"=====(
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>로그인 - Boiler Monitor</title>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background: #f4f7f6; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; }
        .login-box { background: white; padding: 40px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); text-align: center; width: 300px; }
        h2 { color: #2c3e50; font-size: 1.5rem; margin-bottom: 30px; }
        input { width: 100%; padding: 12px; margin: 10px 0; border: 1px solid #ddd; border-radius: 6px; box-sizing: border-box; }
        button { width: 100%; padding: 12px; background: #2c3e50; color: white; border: none; border-radius: 6px; cursor: pointer; font-size: 1rem; transition: background 0.2s; }
        button:hover { background: #34495e; }
    </style>
</head>
<body>
    <div class="login-box">
        <h2>System Login</h2>
        <form method="GET" action="/login">
            <input type="password" name="pass" placeholder="Password" required autofocus>
            <button type="submit">Access Dashboard</button>
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
    <title>Boiler Temp Monitor Rev06</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-zoom"></script>
    <script src="https://cdn.jsdelivr.net/npm/chartjs-plugin-annotation@2.1.0"></script>
    <style>
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background-color: #f4f7f6; margin: 0; display: flex; flex-direction: column; align-items: center; }
        header { background-color: #2c3e50; color: white; width: 100%; padding: 1rem 0; text-align: center; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        .container { max-width: 1000px; width: 95%; margin: 20px auto; display: flex; flex-direction: column; gap: 20px; }
        
        /* Controls & Stats */
        .controls-row { display: flex; flex-wrap: wrap; gap: 10px; justify-content: center; }
        .panel { background: white; padding: 15px; border-radius: 12px; box-shadow: 0 2px 5px rgba(0,0,0,0.05); }
        
        .threshold-controls { display: flex; gap: 15px; align-items: center; flex-wrap: wrap; }
        .threshold-controls input { width: 60px; padding: 5px; border: 1px solid #ccc; border-radius: 4px; text-align: center; }
        
        .event-controls button { background-color: #8e44ad; color: white; border: none; padding: 8px 15px; border-radius: 5px; cursor: pointer; transition: 0.2s; }
        .event-controls button:hover { background-color: #732d91; }

        /* Main Monitoring Groups */
        .monitor-group { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; background: #fff; padding: 20px; border-radius: 15px; box-shadow: 0 4px 6px rgba(0,0,0,0.05); margin-bottom: 20px; }
        .monitor-header { grid-column: span 2; display: flex; justify-content: space-between; align-items: center; border-bottom: 2px solid #ecf0f1; padding-bottom: 10px; margin-bottom: 10px; }
        .monitor-header h2 { margin: 0; color: #2c3e50; font-size: 1.2rem; }
        
        /* Sensor Cards */
        .card-container { grid-column: span 2; display: flex; gap: 10px; justify-content: center; }
        .card { background: #f8f9fa; padding: 15px; border-radius: 10px; text-align: center; min-width: 100px; border: 1px solid #eee; }
        .card h3 { margin: 0 0 5px 0; color: #7f8c8d; font-size: 0.8rem; text-transform: uppercase; }
        .card .value { font-size: 1.8rem; font-weight: bold; color: #2c3e50; }
        .card .unit { font-size: 0.8rem; color: #95a5a6; }
        
        .card.delta { background: #eaf2f8; border-color: #aed6f1; }
        .card.delta .value { color: #2980b9; }

        /* Stats Widget */
        .stats-widget { display: flex; gap: 15px; grid-column: span 2; background: #fafafa; padding: 10px; border-radius: 8px; justify-content: space-around; font-size: 0.9rem; color: #555; }
        .stat-item span { font-weight: bold; color: #333; }

        /* Chart Areas */
        .chart-wrapper { grid-column: span 2; position: relative; height: 300px; width: 100%; }
        .chart-wrapper.small { height: 150px; border-top: 1px dashed #ddd; margin-top: 10px; padding-top: 10px; }

        /* Alerts */
        .alert-active { animation: blink-bg 2s infinite; }
        @keyframes blink-bg {
            0% { background-color: #fff; }
            50% { background-color: #fadbd8; }
            100% { background-color: #fff; }
        }

        /* Responsive */
        @media (max-width: 768px) {
            .monitor-group { display: flex; flex-direction: column; }
        }
    </style>
</head>
<body>
    <header>
        <h1>Boiler Monitor Rev06</h1>
    </header>
    
    <div class="container">
        <!-- Top Controls -->
        <div class="panel controls-row">
            <div class="control-item">
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
                </select>
            </div>
            <div class="control-item">
                 <input type="checkbox" id="auto-scroll" checked> <label for="auto-scroll">자동 스크롤 (Auto-Follow)</label>
            </div>

            <div class="threshold-controls">
                <label>경고(dT):</label>
                <input type="number" id="warn-limit" value="10.0" step="0.5" style="width:50px;">
                <label>위험(dT):</label>
                <input type="number" id="danger-limit" value="20.0" step="0.5" style="width:50px;">
            </div>
            
            <div class="event-controls">
                <button onclick="addEventAnnotation('가동')">가동</button>
                <button onclick="addEventAnnotation('정지')">정지</button>
                <button onclick="resetZoom()" style="background-color: #95a5a6;">줌 초기화</button>
            </div>
        </div>

        <!-- Group 1: S1 & S2 -->
        <div class="monitor-group" id="group1">
            <div class="monitor-header">
                <h2>그룹 1 (S1 - S2)</h2>
                <div class="status-indicator" id="status1">정상</div>
            </div>
            
            <div class="card-container">
                <div class="card"><h3>Temp 1</h3><div class="value" id="s1">--.-</div><span class="unit">°C</span></div>
                <div class="card"><h3>Temp 2</h3><div class="value" id="s2">--.-</div><span class="unit">°C</span></div>
                <div class="card delta"><h3>Delta T</h3><div class="value" id="dt1">--.-</div><span class="unit">°C</span></div>
            </div>

            <div class="stats-widget">
                <div class="stat-item">Max dT: <span id="max-dt1">0.0</span></div>
                <div class="stat-item">Min dT: <span id="min-dt1">0.0</span></div>
                <div class="stat-item">Avg dT: <span id="avg-dt1">0.0</span></div>
            </div>

            <div class="chart-wrapper">
                <canvas id="chart1_main"></canvas>
            </div>
            <div class="chart-wrapper small">
                <!-- Dedicated Delta Chart -->
                <canvas id="chart1_delta"></canvas>
            </div>
        </div>

        <!-- Group 2: S3 & S4 -->
        <div class="monitor-group" id="group2">
            <div class="monitor-header">
                <h2>그룹 2 (S3 - S4)</h2>
                <div class="status-indicator" id="status2">정상</div>
            </div>

            <div class="card-container">
                <div class="card"><h3>Temp 3</h3><div class="value" id="s3">--.-</div><span class="unit">°C</span></div>
                <div class="card"><h3>Temp 4</h3><div class="value" id="s4">--.-</div><span class="unit">°C</span></div>
                <div class="card delta"><h3>Delta T</h3><div class="value" id="dt2">--.-</div><span class="unit">°C</span></div>
            </div>

            <div class="stats-widget">
                <div class="stat-item">Max dT: <span id="max-dt2">0.0</span></div>
                <div class="stat-item">Min dT: <span id="min-dt2">0.0</span></div>
                <div class="stat-item">Avg dT: <span id="avg-dt2">0.0</span></div>
            </div>

            <div class="chart-wrapper">
                <canvas id="chart2_main"></canvas>
            </div>
            <div class="chart-wrapper small">
                <canvas id="chart2_delta"></canvas>
            </div>
        </div>
        
        <div class="panel" style="text-align: center; color: #7f8c8d; font-size: 0.8rem;">
            상태: <span id="connection-status" class="online">연결됨</span> | 
            PC 기록: <span id="logging-status">--</span> | 
            마지막 갱신: <span id="last-update">--:--:--</span>
        </div>
    </div>

    <script>
        const MAX_POINTS = 300; // 포인트 저장 개수 증가 (2초 * 300 = 600초 = 10분 버퍼)
        let updateTimer;
        let charts = {};
        let stats = {
            g1: { max: 0, min: 999, sSum: 0, count: 0 },
            g2: { max: 0, min: 999, sSum: 0, count: 0 }
        };

        const chartCommonOptions = {
            responsive: true,
            maintainAspectRatio: false,
            interaction: { mode: 'index', intersect: false },
            scales: {
                x: {
                    ticks: {
                        autoSkip: true,
                        maxRotation: 0,
                        maxTicksLimit: 10
                    }
                }
            },
            plugins: {
                zoom: { 
                    zoom: { wheel: { enabled: true }, pinch: { enabled: true }, mode: 'x' },
                    pan: { enabled: true, mode: 'x' }
                },
                annotation: { annotations: {} } // For event tagging
            },
            animation: { duration: 0 }
        };

        function initCharts() {
            // Group 1 Main (Overlay)
            charts.c1m = new Chart(document.getElementById('chart1_main'), {
                type: 'line',
                data: { labels: [], datasets: [
                    { label: 'S1', borderColor: '#e74c3c', backgroundColor: 'rgba(231, 76, 60, 0.1)', data: [], fill: false },
                    { label: 'S2', borderColor: '#3498db', backgroundColor: 'rgba(52, 152, 219, 0.2)', data: [], fill: '-1' } // Fill to previous dataset (S1)
                ]},
                options: { ...chartCommonOptions, scales: { y: { suggestedMin: 20, suggestedMax: 80 } } }
            });

            // Group 1 Delta (Trend)
            charts.c1d = new Chart(document.getElementById('chart1_delta'), {
                type: 'line',
                data: { labels: [], datasets: [
                    { label: 'Delta (S1-S2)', borderColor: '#8e44ad', backgroundColor: 'rgba(142, 68, 173, 0.2)', data: [], fill: true }
                ]},
                options: { ...chartCommonOptions, scales: { y: { beginAtZero: true } } }
            });

            // Group 2 Main
            charts.c2m = new Chart(document.getElementById('chart2_main'), {
                type: 'line',
                data: { labels: [], datasets: [
                    { label: 'S3', borderColor: '#2ecc71', backgroundColor: 'rgba(46, 204, 113, 0.1)', data: [], fill: false },
                    { label: 'S4', borderColor: '#f1c40f', backgroundColor: 'rgba(241, 196, 15, 0.2)', data: [], fill: '-1' }
                ]},
                options: { ...chartCommonOptions, scales: { y: { suggestedMin: 20, suggestedMax: 80 } } }
            });

            // Group 2 Delta
            charts.c2d = new Chart(document.getElementById('chart2_delta'), {
                type: 'line',
                data: { labels: [], datasets: [
                    { label: 'Delta (S3-S4)', borderColor: '#e67e22', backgroundColor: 'rgba(230, 126, 34, 0.2)', data: [], fill: true }
                ]},
                options: { ...chartCommonOptions, scales: { y: { beginAtZero: true } } }
            });
        }

        function updateStats(group, currentDelta) {
            const s = stats[group];
            if (currentDelta > s.max) s.max = currentDelta;
            if (currentDelta < s.min) s.min = currentDelta;
            s.sSum += currentDelta;
            s.count++;
            
            const avg = s.sSum / s.count;
            
            if(group === 'g1') {
                document.getElementById('max-dt1').innerText = s.max.toFixed(1);
                document.getElementById('min-dt1').innerText = s.min.toFixed(1);
                document.getElementById('avg-dt1').innerText = avg.toFixed(1);
            } else {
                document.getElementById('max-dt2').innerText = s.max.toFixed(1);
                document.getElementById('min-dt2').innerText = s.min.toFixed(1);
                document.getElementById('avg-dt2').innerText = avg.toFixed(1);
            }
        }

        function checkThresholds(id, groupElem, delta) {
            const warn = parseFloat(document.getElementById('warn-limit').value) || 10;
            const danger = parseFloat(document.getElementById('danger-limit').value) || 20;
            const statusEl = groupElem.querySelector('.status-indicator');

            if (delta >= danger) {
                groupElem.classList.add('alert-active');
                statusEl.innerText = "위험 (Danger)";
                statusEl.style.color = "#c0392b";
                statusEl.style.fontWeight = "bold";
            } else if (delta >= warn) {
                groupElem.classList.remove('alert-active');
                groupElem.style.background = "#fff3cd"; // Yellow tint
                statusEl.innerText = "주의 (Warning)";
                statusEl.style.color = "#d35400";
                statusEl.style.fontWeight = "bold";
            } else {
                groupElem.classList.remove('alert-active');
                groupElem.style.background = "#fff";
                statusEl.innerText = "정상";
                statusEl.style.color = "#27ae60";
            }
        }

        function addEventAnnotation(label) {
            const now = new Date().toLocaleTimeString();
            const note = {
                type: 'line',
                scaleID: 'x',
                value: now,
                borderColor: 'black',
                borderWidth: 2,
                label: {
                    content: label,
                    display: true,
                    position: 'start'
                }
            };

            // Add to all charts
            [charts.c1m, charts.c1d, charts.c2m, charts.c2d].forEach(c => {
                if(!c) return;
                const id = 'evt_' + Date.now();
                if (!c.options.plugins.annotation.annotations) c.options.plugins.annotation.annotations = {};
                c.options.plugins.annotation.annotations[id] = note;
                c.update();
            });
        }


        function changeInterval(ms) {
            clearInterval(updateTimer);
            updateTimer = setInterval(updateData, parseInt(ms));
        }

        function resetZoom() {
            Object.values(charts).forEach(c => c.resetZoom());
        }

        function updateData() {
            fetch('/data')
                .then(res => res.json())
                .then(data => {
                    const now = new Date().toLocaleTimeString('ko-KR', {hour12: false});
                    
                    // Update Text
                    ['s1', 's2', 's3', 's4'].forEach(k => {
                        if(data[k] !== undefined) document.getElementById(k).innerText = data[k].toFixed(1);
                    });

                    const d1 = (data.s1 !== undefined && data.s2 !== undefined) ? Math.abs(data.s1 - data.s2) : 0;
                    const d2 = (data.s3 !== undefined && data.s4 !== undefined) ? Math.abs(data.s3 - data.s4) : 0;
                    
                    document.getElementById('dt1').innerText = d1.toFixed(1);
                    document.getElementById('dt2').innerText = d2.toFixed(1);

                    // Update Stats
                    updateStats('g1', d1);
                    updateStats('g2', d2);

                    // Check Thresholds
                    checkThresholds(1, document.getElementById('group1'), d1);
                    checkThresholds(2, document.getElementById('group2'), d2);

                    // Update Charts
                    const pushData = (chart, label, ...values) => {
                        if (chart.data.labels.length > MAX_POINTS) {
                            chart.data.labels.shift();
                            chart.data.datasets.forEach(ds => ds.data.shift());
                        }
                        chart.data.labels.push(label);
                        values.forEach((v, i) => chart.data.datasets[i].data.push(v));
                        
                        // Auto Scroll Logic
                        if (document.getElementById('auto-scroll').checked) {
                            chart.resetZoom(); 
                        }
                        chart.update('none');
                    };

                    if(charts.c1m) pushData(charts.c1m, now, data.s1, data.s2);
                    if(charts.c1d) pushData(charts.c1d, now, d1);
                    if(charts.c2m) pushData(charts.c2m, now, data.s3, data.s4);
                    if(charts.c2d) pushData(charts.c2d, now, d2);
                    
                    document.getElementById('last-update').innerText = now;
                    document.getElementById('connection-status').innerText = "연결됨";

                    // Logging status
                     const logStatus = document.getElementById('logging-status');
                    if (data.logging) {
                        logStatus.innerText = "ON";
                        logStatus.style.color = "green";
                    } else {
                        logStatus.innerText = "OFF";
                        logStatus.style.color = "red";
                    }
                })
                .catch(e => console.error(e));
        }

        window.onload = () => {
            initCharts();
            updateTimer = setInterval(updateData, 2000);
            updateData();
        };
    </script>
</body>
</html>
)=====";

#endif
