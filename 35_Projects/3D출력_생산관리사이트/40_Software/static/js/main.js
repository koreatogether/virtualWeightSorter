document.addEventListener('DOMContentLoaded', () => {
    // 테마 토글
    document.getElementById('theme-toggle').addEventListener('click', () => {
        document.body.classList.toggle('dark-mode');
    });

    // --- Flatpickr 초기화 ---
    let datePicker = flatpickr("#start_date_picker", {
        enableTime: true,
        dateFormat: "Y-m-d H:i",
        time_24hr: true,
        minDate: "today",
        disable: [] // 초기엔 비활성 날짜 없음
    });

    // 프린터 선택 시 바쁜 날짜 가져오기
    const printerSelect = document.getElementById('printer_id');
    printerSelect.addEventListener('change', async (e) => {
        const printerId = e.target.value;
        if (printerId) {
            await updateBusyDates(printerId);
        } else {
            datePicker.set('disable', []);
        }
    });

    async function updateBusyDates(printerId) {
        try {
            const res = await fetch(`/api/v1/printers/${printerId}/busy_dates`);
            if (res.ok) {
                const busyDates = await res.json();
                // Flatpickr에 비활성 날짜 설정
                datePicker.set('disable', busyDates);
            }
        } catch (err) {
            console.error('Failed to fetch busy dates', err);
        }
    }

    // 자동 계획 모드 토글
    const autoModeCheck = document.getElementById('auto_schedule_mode');
    const scheduleBtn = document.getElementById('schedule-btn');

    autoModeCheck.addEventListener('change', (e) => {
        const isAuto = e.target.checked;
        if (isAuto) {
            scheduleBtn.textContent = '자동 계획 생성 (일괄 등록)';
            // 자동 모드는 날짜만 필요
            datePicker.set('enableTime', false);
            datePicker.set('dateFormat', "Y-m-d");
        } else {
            scheduleBtn.textContent = '작업 할당';
            // 수동 모드는 시간까지 필요
            datePicker.set('enableTime', true);
            datePicker.set('dateFormat', "Y-m-d H:i");
        }
    });

    // 1. 주문 생성 및 수정
    document.getElementById('order-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            product_name: document.getElementById('order_product').value,
            target_quantity: parseInt(document.getElementById('order_target').value),
            initial_stock: parseInt(document.getElementById('order_initial_stock').value || 0),
            material: document.getElementById('order_material').value,
            color: document.getElementById('order_color').value,
            unit_weight_g: parseFloat(document.getElementById('order_unit_weight').value || 0),
            deadline: document.getElementById('order_deadline').value
        };
        const res = await postData('/api/v1/orders', payload);
        if (res.ok) { await fetchData(); e.target.reset(); }
    });

    document.getElementById('order-edit-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('edit_order_id').value;
        const payload = {
            product_name: document.getElementById('edit_order_product').value,
            target_quantity: parseInt(document.getElementById('edit_order_target').value),
            initial_stock: parseInt(document.getElementById('edit_order_initial_stock').value || 0),
            material: document.getElementById('edit_order_material').value,
            color: document.getElementById('edit_order_color').value,
            unit_weight_g: parseFloat(document.getElementById('edit_order_unit_weight').value),
            deadline: document.getElementById('edit_order_deadline').value
        };
        const res = await postData(`/api/v1/orders/${id}`, payload, 'PATCH');
        if (res.ok) { closeOrderEdit(); await fetchData(); }
    });

    // 2. 재고 생성 및 수정
    document.getElementById('inventory-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            material: document.getElementById('material').value, color: document.getElementById('color').value,
            remaining_weight_g: parseFloat(document.getElementById('remaining_weight_g').value)
        };
        if ((await postData('/api/v1/inventory', payload)).ok) { await fetchData(); e.target.reset(); }
    });

    document.getElementById('inventory-edit-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('edit_inv_id').value;
        const payload = {
            material: document.getElementById('edit_inv_material').value,
            color: document.getElementById('edit_inv_color').value,
            remaining_weight_g: parseFloat(document.getElementById('edit_inv_weight').value)
        };
        if ((await postData(`/api/v1/inventory/${id}`, payload, 'PATCH')).ok) { closeInventoryEdit(); await fetchData(); }
    });

    // 3. 프린터 생성 및 수정
    document.getElementById('printer-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const payload = {
            model: document.getElementById('printer_model').value,
            asset_id: document.getElementById('printer_asset_id').value,
            purchase_date: document.getElementById('purchase_date').value
        };
        if ((await postData('/api/v1/printers', payload)).ok) { await fetchData(); e.target.reset(); }
    });

    document.getElementById('printer-edit-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('edit_printer_id').value;
        const payload = {
            model: document.getElementById('edit_printer_model').value,
            asset_id: document.getElementById('edit_printer_asset_id').value,
            purchase_date: document.getElementById('edit_printer_purchase').value
        };
        if ((await postData(`/api/v1/printers/${id}`, payload, 'PATCH')).ok) { closePrinterEdit(); await fetchData(); }
    });

    // 4. 생산 작업 할당 (Schedule)
    document.getElementById('schedule-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const isAuto = document.getElementById('auto_schedule_mode').checked;
        const orderId = document.getElementById('schedule_order_id').value;
        const printerId = document.getElementById('printer_id').value;
        const plannedQty = parseInt(document.getElementById('planned_qty').value);
        const dateValue = document.getElementById('start_date_picker').value;

        if (!dateValue) return alert("날짜를 선택해주세요.");

        if (isAuto) {
            // dateValue: YYYY-MM-DD
            const payload = { order_id: orderId, printer_id: printerId, daily_qty: plannedQty, start_date: dateValue };
            const res = await postData('/api/v1/schedules/auto', payload);
            if (res.ok) { 
                const data = await res.json();
                let msg = `${data.count}일치 생산 계획이 자동 생성되었습니다.`;
                if (data.conflicts && data.conflicts.length > 0) msg += `\n[경고] 날짜 중복 감지: ${data.conflicts.join(', ')}`;
                alert(msg); 
                // 해당 프린터의 바쁜 날짜 갱신
                await updateBusyDates(printerId);
                await fetchData(); 
                e.target.reset();
            } else { const err = await res.json(); alert('오류: ' + err.message); }
        } else {
            // dateValue: YYYY-MM-DD HH:mm (T 추가 필요)
            const isoTime = dateValue.replace(' ', 'T');
            const payload = { order_id: orderId, printer_id: printerId, planned_quantity: plannedQty, start_time: isoTime };
            const res = await postData('/api/v1/schedules', payload);
            if (res.ok) { 
                const data = await res.json(); 
                if (data.conflict) alert("경고: 중복 예약 감지!"); 
                await updateBusyDates(printerId);
                await fetchData(); 
                e.target.reset(); 
            }
        }
    });

    fetchData();
});

let currentData = null;

async function fetchData() {
    try {
        const response = await fetch('/api/v1/data');
        currentData = await response.json();
        renderOrders(currentData.orders || [], currentData.schedules || []);
        renderSchedules(currentData.schedules || [], currentData.orders || [], currentData.printers || []);
        renderDashboard(currentData);
        renderInventory(currentData.inventory || [], currentData.orders || []);
        renderPrinters(currentData.printers || []);
        updateSelects(currentData.orders || [], currentData.printers || []);
        updateFilamentSelects(currentData.inventory || []);
        renderTimeline(currentData);
    } catch (error) { console.error('Error:', error); }
}

// --- Render Logic ---

function renderOrders(orders, schedules) {
    const container = document.getElementById('order-list');
    container.innerHTML = '';
    orders.forEach(order => {
        const relatedSchedules = schedules.filter(s => s.order_id === order.id);
        const actualSum = relatedSchedules.reduce((sum, s) => sum + (parseInt(s.actual_quantity) || 0), 0);
        const initialStock = parseInt(order.initial_stock || 0);
        const totalSecured = initialStock + actualSum;
        const progress = order.target_quantity ? Math.min(100, Math.floor((totalSecured / order.target_quantity) * 100)) : 0;
        const remaining = order.target_quantity - totalSecured;
        const estimatedUsage = (order.unit_weight_g || 0) * (Math.max(0, remaining));

        const card = document.createElement('div');
        card.className = 'order-card';
        card.innerHTML = `
            <div class="order-header">
                <h3>${order.product_name}</h3>
                <div>
                    <button onclick="openOrderEdit('${order.id}')" class="btn-sm" style="background:#ffc107; color:#333;">수정</button>
                    <span class="badge ${order.status}">${order.status}</span>
                </div>
            </div>
            <div class="progress-bar-bg">
                <div class="progress-bar-fill" style="width: ${progress}%"></div>
            </div>
            <div class="order-stats">
                <span>목표: ${order.target_quantity}</span>
                <span>확보: <strong>${totalSecured}</strong></span>
                <span>잔여: <span style="color: ${remaining > 0 ? 'red' : 'green'}">${Math.max(0, remaining)}</span></span>
            </div>
            <div style="font-size: 0.85em; color: #555; margin-top: 5px;">
                사용: ${order.material}/${order.color} (${order.unit_weight_g}g/ea)<br>
                필요 필라멘트: <strong>${estimatedUsage.toLocaleString()}g</strong>
            </div>
            <div style="text-align: right; margin-top: 5px; font-size: 0.85em; color: #666;">
                마감: ${order.deadline || '없음'} 
                <button onclick="deleteOrder('${order.id}')" class="btn-sm btn-danger">삭제</button>
            </div>
        `;
        container.appendChild(card);
    });
}

function openOrderEdit(id) {
    const order = currentData.orders.find(o => o.id === id);
    if (!order) return;
    updateFilamentSelects(currentData.inventory || []);
    document.getElementById('edit_order_id').value = order.id;
    document.getElementById('edit_order_product').value = order.product_name;
    document.getElementById('edit_order_target').value = order.target_quantity;
    document.getElementById('edit_order_initial_stock').value = order.initial_stock || 0;
    document.getElementById('edit_order_material').value = order.material || '';
    document.getElementById('edit_order_color').value = order.color || '';
    document.getElementById('edit_order_unit_weight').value = order.unit_weight_g || 0;
    document.getElementById('edit_order_deadline').value = order.deadline || '';
    document.getElementById('order-edit-overlay').style.display = 'block';
}
function closeOrderEdit() { document.getElementById('order-edit-overlay').style.display = 'none'; }

function updateFilamentSelects(inventory) {
    const materialSelects = [document.getElementById('order_material'), document.getElementById('edit_order_material')];
    const colorSelects = [document.getElementById('order_color'), document.getElementById('edit_order_color')];
    const materials = [...new Set(inventory.map(i => i.material))].sort();
    const colors = [...new Set(inventory.map(i => i.color))].sort();
    materialSelects.forEach(sel => { if (!sel) return; const cur = sel.value; sel.innerHTML = '<option value="">-- 재질 선택 --</option>' + materials.map(m => `<option value="${m}">${m}</option>`).join(''); if (cur) sel.value = cur; });
    colorSelects.forEach(sel => { if (!sel) return; const cur = sel.value; sel.innerHTML = '<option value="">-- 색상 선택 --</option>' + colors.map(c => `<option value="${c}">${c}</option>`).join(''); if (cur) sel.value = cur; });
}

function renderSchedules(schedules, orders, printers) {
    const tbody = document.getElementById('schedule-body');
    tbody.innerHTML = '';
    const sortedSchedules = schedules.slice().sort((a, b) => new Date(a.start_time) - new Date(b.start_time));
    let lastOrderId = null;
    sortedSchedules.forEach(s => {
        const order = orders.find(o => o.id === s.order_id) || { product_name: 'Unknown' };
        const printer = printers.find(p => p.id === s.printer_id) || { model: 'Unknown', asset_id: 'N/A' };
        if (lastOrderId !== null && lastOrderId !== s.order_id) {
            const separator = document.createElement('tr');
            separator.className = 'product-separator';
            separator.innerHTML = `<td colspan="6" style="background:#eee; height:10px; padding:0;"></td>`;
            tbody.appendChild(separator);
        }
        const tr = document.createElement('tr');
        if (s.conflict) tr.className = 'conflict-row';
        tr.innerHTML = `
            <td><strong>${s.start_time ? s.start_time.split('T')[0] : '-'}</strong></td>
            <td>
                <div style="font-weight:bold;">${order.product_name}</div>
                <div style="font-size:0.8em; color:#666;">${order.material}/${order.color}</div>
            </td>
            <td>${printer.model} <br><small>(${printer.asset_id})</small></td>
            <td>
                <div style="color:var(--accent-color)">계획: ${s.planned_quantity}</div>
                <div style="font-weight:bold;">실적: ${s.actual_quantity}</div>
            </td>
            <td>
                <div style="display: flex; gap: 5px; justify-content: center; align-items: center;">
                    <button onclick="updateRun('${s.id}', prompt('추가 성공 수량 입력 (누적됨):', '0'))" class="btn-sm">실적입력</button>
                    <button onclick="deleteSchedule('${s.id}')" class="btn-sm btn-danger">삭제</button>
                </div>
            </td>
            <td><small style="color:#999;">${s.id}</small></td>
        `;
        tbody.appendChild(tr);
        lastOrderId = s.order_id;
    });
}

async function updateRun(id, qtyVal) {
    if (qtyVal === null) return;
    await postData(`/api/v1/schedules/${id}`, { actual_quantity: parseInt(qtyVal), status: 'completed' }, 'PATCH');
    fetchData();
}

async function postData(url, data, method='POST') { return fetch(url, { method: method, headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data) }); }
async function deleteOrder(id) { if(confirm('주문을 삭제하시겠습니까?')) { await fetch(`/api/v1/orders/${id}`, { method: 'DELETE' }); fetchData(); } }
async function deleteSchedule(id) { if (confirm('계획 삭제?')) { await fetch(`/api/v1/schedules/${id}`, { method: 'DELETE' }); fetchData(); } }
async function deletePrinter(id) { if(confirm('프린터 삭제?')) { await fetch(`/api/v1/printers/${id}`, { method: 'DELETE' }); fetchData(); } }
async function deleteInventory(id) { if(confirm('재고 삭제?')) { await fetch(`/api/v1/inventory/${id}`, { method: 'DELETE' }); fetchData(); } }

function updateSelects(orders, printers) {
    const os = document.getElementById('schedule_order_id');
    const ps = document.getElementById('printer_id');
    const curP = ps.value;
    if (orders.length) os.innerHTML = '<option value="">-- 주문 선택 --</option>' + orders.filter(o => o.status !== 'completed').map(o => `<option value="${o.id}">${o.product_name}</option>`).join('');
    if (printers.length) ps.innerHTML = '<option value="">-- 프린터 선택 --</option>' + printers.map(p => `<option value="${p.id}">${p.model} (${p.asset_id})</option>`).join('');
    if (curP) ps.value = curP;
}

// --- Helper Logic ---
function getColorStyle(colorName) {
    const colorMap = {
        '백색': '#ffffff', '화이트': '#ffffff', 'white': '#ffffff',
        '검정': '#000000', '블랙': '#000000', 'black': '#000000',
        '회색': '#808080', '그레이': '#808080', 'grey': '#808080', 'gray': '#808080',
        '빨강': '#ff0000', '레드': '#ff0000', 'red': '#ff0000',
        '파랑': '#0000ff', '블루': '#0000ff', 'blue': '#0000ff',
        '녹색': '#28a745', '그린': '#28a745', 'green': '#28a745',
        '노랑': '#ffc107', '옐로우': '#ffc107', 'yellow': '#ffc107',
        '오렌지': '#fd7e14', 'orange': '#fd7e14',
        '보라': '#6f42c1', 'purple': '#6f42c1',
        '투명': '#e9ecef', 'clear': '#e9ecef'
    };

    const bgColor = colorMap[colorName] || '#dee2e6'; // 매핑 안되면 연한 회색

    // 밝기 계산 (Luma: 0.299R + 0.587G + 0.114B)
    // HEX를 RGB로 변환하여 계산
    let r, g, b;
    if (bgColor.startsWith('#')) {
        const hex = bgColor.slice(1);
        r = parseInt(hex.slice(0, 2), 16);
        g = parseInt(hex.slice(2, 4), 16);
        b = parseInt(hex.slice(4, 6), 16);
    } else {
        r = g = b = 200;
    }
    
    const brightness = (r * 299 + g * 587 + b * 114) / 1000;
    const textColor = brightness > 128 ? '#000000' : '#ffffff';

    return `background-color: ${bgColor}; color: ${textColor}; padding: 2px 8px; border-radius: 4px; border: 1px solid #ddd; font-weight: bold;`;
}

function renderInventory(inv, orders) { 
    const tb = document.getElementById('inventory-body'); tb.innerHTML = '';
    
    // 1. 재질/색상별 데이터 집계
    const stats = {}; 

    const getStats = (m, c) => {
        const key = `${m}_${c}`;
        if (!stats[key]) stats[key] = { stock: 0, estimatedCurrentStock: 0, future: 0 };
        return stats[key];
    };

    // 재고 기반으로 현재 추정 잔량 계산
    inv.forEach(i => {
        const s = getStats(i.material, i.color);
        s.stock += parseFloat(i.remaining_weight_g || 0);
        
        // 이 특정 스풀(i)의 업데이트 시간
        const lastUpdate = i.updated_at ? new Date(i.updated_at) : new Date(0);
        
        // 이 스풀의 재질/색상을 사용하는 실적 중, 업데이트 시간 이후의 것만 소모량으로 계산
        const consumedAfterUpdate = (currentData.schedules || [])
            .filter(sch => {
                const order = (currentData.orders || []).find(o => o.id === sch.order_id);
                if (!order || order.material !== i.material || order.color !== i.color) return false;
                
                // 실적 입력/생성 시간 (created_at 혹은 별도 필드 없으므로 일단 created_at 기준, 
                // 더 정확하게는 실적 업데이트 시간을 기록해야 하지만 현재 구조에서 최선책 적용)
                const actionTime = sch.created_at ? new Date(sch.created_at) : new Date(0);
                return actionTime > lastUpdate;
            })
            .reduce((sum, sch) => sum + (parseInt(sch.actual_quantity) || 0), 0);
        
        // 이 스풀에서 추정되는 현재 잔량 = 입력 무게 - 입력 이후의 소모량
        const itemEstimatedStock = parseFloat(i.remaining_weight_g || 0) - (consumedAfterUpdate * (
            (currentData.orders.find(o => o.material === i.material && o.color === i.color) || {}).unit_weight_g || 0
        ));
        s.estimatedCurrentStock += itemEstimatedStock;
    });

    // 향후 필요량 계산 (이건 시간과 상관없이 전체 잔여량 기준)
    orders.filter(o => o.status !== 'completed').forEach(o => {
        const s = getStats(o.material, o.color);
        const actualSum = (currentData.schedules||[]).filter(sch => sch.order_id === o.id).reduce((sum, sch) => sum + (parseInt(sch.actual_quantity)||0), 0);
        const remainingQty = Math.max(0, o.target_quantity - o.initial_stock - actualSum);
        s.future += remainingQty * (o.unit_weight_g || 0);
    });

    // 2. 렌더링 (재질명, 색상명 순으로 정렬 추가)
    const sortedInv = inv.slice().sort((a, b) => {
        // 재질명으로 먼저 비교
        const matCompare = a.material.localeCompare(b.material, undefined, {numeric: true, sensitivity: 'base'});
        if (matCompare !== 0) return matCompare;
        // 재질명이 같으면 색상명으로 비교
        return a.color.localeCompare(b.color, undefined, {numeric: true, sensitivity: 'base'});
    });

    sortedInv.forEach(i => {
        const key = `${i.material}_${i.color}`;
        const s = stats[key];
        
        // 화면 표시용: 개별 아이템의 추정 잔량 다시 계산 (위 로직 반복)
        const lastUpdate = i.updated_at ? new Date(i.updated_at) : new Date(0);
        const orderForWeight = currentData.orders.find(o => o.material === i.material && o.color === i.color) || {unit_weight_g:0};
        const consumedAfter = (currentData.schedules || [])
            .filter(sch => {
                const order = (currentData.orders || []).find(o => o.id === sch.order_id);
                return order && order.material === i.material && order.color === i.color && (sch.created_at ? new Date(sch.created_at) : new Date(0)) > lastUpdate;
            })
            .reduce((sum, sch) => sum + (parseInt(sch.actual_quantity) || 0), 0);
        
        const itemEstimatedStock = i.remaining_weight_g - (consumedAfter * orderForWeight.unit_weight_g);
        
        const isShort = s.estimatedCurrentStock < s.future;
        const tr = document.createElement('tr');
        
        if (isShort) tr.className = 'conflict-row';
        
        let statusHtml = '';
        if (s.future > 0) {
            if (isShort) {
                statusHtml = `<br><small style="color:red; font-weight:bold;">부족 (전체추정잔량: ${s.estimatedCurrentStock.toLocaleString()}g / 필요: ${s.future.toLocaleString()}g)</small>`;
            } else {
                const surplus = s.estimatedCurrentStock - s.future;
                statusHtml = `<br><small style="color:green; font-weight:bold;">충분 (전체추정잔량: ${s.estimatedCurrentStock.toLocaleString()}g / 여유: ${surplus.toLocaleString()}g)</small>`;
            }
        }

        tr.innerHTML = `
            <td>${i.material}</td>
            <td><span style="${getColorStyle(i.color)}">${i.color}</span></td>
            <td>
                <span title="등록 무게: ${i.remaining_weight_g}g">${itemEstimatedStock.toLocaleString()}g</span>
                ${statusHtml}
            </td>
            <td><small>${i.updated_at || '-'}</small></td>
            <td>
                <button onclick="openInventoryEdit('${i.id}')" class="btn-sm">수정</button>
                <button onclick="deleteInventory('${i.id}')" class="btn-sm btn-danger">삭제</button>
            </td>`;
        tb.appendChild(tr);
    });
}

function renderPrinters(list) {
    const tb = document.getElementById('printer-body'); tb.innerHTML = '';
    list.forEach(p => {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${p.model}</td><td>${p.asset_id}</td><td>${p.purchase_date}</td>
            <td><button onclick="openPrinterEdit('${p.id}')" class="btn-sm">수정</button><button onclick="deletePrinter('${p.id}')" class="btn-sm btn-danger">삭제</button></td>`;
        tb.appendChild(tr);
    });
}

function renderDashboard(data) {
    const totalTarget = (data.orders||[]).reduce((sum, o) => sum + parseInt(o.target_quantity), 0);
    const totalSecured = (data.orders||[]).reduce((sum, o) => sum + parseInt(o.initial_stock) + parseInt(o.completed_quantity), 0);
    const percent = totalTarget ? Math.floor((totalSecured/totalTarget)*100) : 0;
    document.querySelector('#stat-progress .data-value').textContent = `${percent}%`;
    document.querySelector('#stat-printers .data-value').textContent = (data.printers||[]).length;
}

function renderTimeline(data) {
    const header = document.getElementById('gantt-header');
    const body = document.getElementById('gantt-body');
    if (!header || !body) return;
    
    header.innerHTML = '<th>프린터 \\ 시간</th>';
    body.innerHTML = '';

    // 최근 7일간의 날짜 헤더 생성
    const dates = [];
    for (let i = 0; i < 7; i++) {
        const d = new Date();
        d.setDate(d.getDate() + i);
        const dStr = d.toISOString().split('T')[0];
        dates.push(dStr);
        header.innerHTML += `<th>${dStr}</th>`;
    }

    (data.printers || []).forEach(p => {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td><strong>${p.model}</strong><br><small>${p.asset_id}</small></td>`;
        
        dates.forEach(dStr => {
            const task = (data.schedules || []).find(s => s.printer_id === p.id && s.start_time.startsWith(dStr));
            if (task) {
                const order = (data.orders || []).find(o => o.id === task.order_id);
                const color = task.status === 'completed' ? '#d4edda' : '#fff3cd';
                tr.innerHTML += `<td style="background:${color}; font-size:0.8em;">${order ? order.product_name : '작업'}</td>`;
            } else {
                tr.innerHTML += `<td></td>`;
            }
        });
        body.appendChild(tr);
    });
}

function switchView(viewName) {
    document.getElementById('list-view').style.display = viewName === 'list' ? 'block' : 'none';
    document.getElementById('timeline-view').style.display = viewName === 'timeline' ? 'block' : 'none';
}

function openInventoryEdit(id) {
    const item = currentData.inventory.find(i => i.id === id);
    if (!item) return;
    document.getElementById('edit_inv_id').value = item.id;
    document.getElementById('edit_inv_material').value = item.material;
    document.getElementById('edit_inv_color').value = item.color;
    document.getElementById('edit_inv_weight').value = item.remaining_weight_g;
    document.getElementById('inventory-edit-overlay').style.display = 'block';
}

function closeInventoryEdit() {
    document.getElementById('inventory-edit-overlay').style.display = 'none';
}

function openPrinterEdit(id) {
    const item = currentData.printers.find(p => p.id === id);
    if (!item) return;
    document.getElementById('edit_printer_id').value = item.id;
    document.getElementById('edit_printer_model').value = item.model;
    document.getElementById('edit_printer_asset_id').value = item.asset_id;
    document.getElementById('edit_printer_purchase').value = item.purchase_date;
    document.getElementById('printer-edit-overlay').style.display = 'block';
}

function closePrinterEdit() {
    document.getElementById('printer-edit-overlay').style.display = 'none';
}
