#ifndef DASHBOARD_H
#define DASHBOARD_H

const char LOGIN_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>Login</title><style>body{font-family:sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;background:#f0f2f5;}div{background:white;padding:2rem;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}</style></head>
<body><div><h2>Boiler Monitor Login</h2><form action="/login"><input type="password" name="pass" required><button>Enter</button></form></div></body></html>
)=====";

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>Dashboard</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<style>body{font-family:sans-serif;background:#f4f7f6;margin:0;}header{background:#2c3e50;color:white;padding:1rem;text-align:center;}
.container{max-width:900px;margin:20px auto;padding:10px;}.card{background:white;padding:15px;border-radius:10px;margin-bottom:20px;box-shadow:0 2px 5px rgba(0,0,0,0.05);}
.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px;margin-bottom:15px;}
.val-card{text-align:center;padding:10px;background:#f8f9fa;border-radius:8px;}.val{font-size:1.5rem;font-weight:bold;color:#2c3e50;}</style></head>
<body><header><h1>Boiler Monitor RevNew01</h1></header>
<div class="container"><div class="card"><div class="grid">
<div class="val-card">S1<div class="val" id="s1">--.-</div></div><div class="val-card">S2<div class="val" id="s2">--.-</div></div>
<div class="val-card">S3<div class="val" id="s3">--.-</div></div><div class="val-card">S4<div class="val" id="s4">--.-</div></div></div>
<canvas id="chart"></canvas></div></div>
<script>
let chart;
const MAX_POINTS = 100;
function initChart(){
    const ctx = document.getElementById('chart').getContext('2d');
    chart = new Chart(ctx,{type:'line',data:{labels:[],datasets:[
        {label:'S1',borderColor:'#e74c3c',data:[],fill:false},
        {label:'S2',borderColor:'#3498db',data:[],fill:false},
        {label:'S3',borderColor:'#2ecc71',data:[],fill:false},
        {label:'S4',borderColor:'#f1c40f',data:[],fill:false}
    ]},options:{responsive:true,animation:false}});
}
function updateData(){
    fetch('/data').then(r=>r.json()).then(data=>{
        document.getElementById('s1').innerText=data.s1.toFixed(1);
        document.getElementById('s2').innerText=data.s2.toFixed(1);
        document.getElementById('s3').innerText=data.s3.toFixed(1);
        document.getElementById('s4').innerText=data.s4.toFixed(1);
        const now = new Date().toLocaleTimeString();
        if(chart.data.labels.length > MAX_POINTS){chart.data.labels.shift();chart.data.datasets.forEach(d=>d.data.shift());}
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(data.s1);chart.data.datasets[1].data.push(data.s2);
        chart.data.datasets[2].data.push(data.s3);chart.data.datasets[3].data.push(data.s4);
        chart.update();
    });
}
window.onload=()=>{initChart();setInterval(updateData,2000);updateData();};
</script></body></html>
)=====";

#endif
