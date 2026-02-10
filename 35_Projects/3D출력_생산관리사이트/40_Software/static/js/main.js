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
            unit_weight_g: parseInt(document.getElementById('order_unit_weight').value || 0),
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
            unit_weight_g: parseInt(document.getElementById('edit_order_unit_weight').value),
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
            remaining_weight_g: parseInt(document.getElementById('remaining_weight_g').value)
        };
        if ((await postData('/api/v1/inventory', payload)).ok) { await fetchData(); e.target.reset(); }
    });

    document.getElementById('inventory-edit-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('edit_inv_id').value;
        const payload = {
            material: document.getElementById('edit_inv_material').value,
            color: document.getElementById('edit_inv_color').value,
            remaining_weight_g: parseInt(document.getElementById('edit_inv_weight').value)
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
                추가 필요량: <strong>${estimatedUsage.toLocaleString()}g</strong>
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
                    <button onclick="updateRun('${s.id}', prompt('실제 성공 수량 입력:', ${s.planned_quantity}))" class="btn-sm">실적입력</button>
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

function renderInventory(inv, orders) { 
    const tb = document.getElementById('inventory-body'); tb.innerHTML = '';
    inv.forEach(i => {
        const requiredWeight = orders.filter(o => o.status !== 'completed' && o.material === i.material && o.color === i.color).reduce((sum, o) => {
             const actualSum = (currentData.schedules||[]).filter(s => s.order_id === o.id).reduce((s, sch) => s + (parseInt(sch.actual_quantity)||0), 0);
             const remainingQty = o.target_quantity - o.initial_stock - actualSum;
             return sum + (Math.max(0, remainingQty) * (o.unit_weight_g || 0));
        }, 0);
        const isLow = i.remaining_weight_g < requiredWeight;
        const tr = document.createElement('tr');
        if (isLow) tr.className = 'conflict-row';
        tr.innerHTML = `<td>${i.material}</td><td>${i.color}</td><td>${i.remaining_weight_g}g ${isLow ? `<br><small style="color:red;">(필요: ${requiredWeight}g)</small>` : ''}</td>
            <td><button onclick="openInventoryEdit('${i.id}')" class="btn-sm">수정</button><button onclick="deleteInventory('${i.id}')" class="btn-sm btn-danger">삭제</button></td>`;
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

function renderTimeline(data) { /* ... */ }
function switchView(viewName) { /* ... */ }
function openInventoryEdit(id) { /* ... */ }
function closeInventoryEdit() { /* ... */ }
function openPrinterEdit(id) { /* ... */ }
function closePrinterEdit() { /* ... */ }
