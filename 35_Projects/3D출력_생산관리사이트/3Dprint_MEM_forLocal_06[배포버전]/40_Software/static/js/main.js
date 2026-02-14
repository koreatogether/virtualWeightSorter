document.addEventListener('DOMContentLoaded', () => {
    initOrderFormEvents();
    // 테마 토글
    document.getElementById('theme-toggle').addEventListener('click', () => {
        document.body.classList.toggle('dark-mode');
    });

    // 공장 초기화 이벤트
    document.getElementById('factory-reset-btn').addEventListener('click', async () => {
        const warning = "주의: 모든 데이터(주문, 일정, 재고, 프린터)가 영구적으로 삭제됩니다.\n이 작업은 되돌릴 수 없습니다.";
        if (confirm(warning)) {
            const userInput = prompt("초기화를 확정하려면 'idodelete'를 입력하세요:");
            if (userInput === 'idodelete') {
                try {
                    const res = await fetch('/api/v1/system/reset', { method: 'POST' });
                    if (res.ok) {
                        alert("시스템이 성공적으로 초기화되었습니다.");
                        location.reload(); // 페이지 새로고침하여 초기 상태 반영
                    }
                } catch (err) {
                    console.error("Reset failed", err);
                    alert("초기화 중 오류가 발생했습니다.");
                }
            } else if (userInput !== null) {
                alert("입력 문구가 일치하지 않습니다. 작업을 취소합니다.");
            }
        }
    });

    // --- Flatpickr 초기화 ---
    let datePicker = flatpickr("#start_date_picker", {
        enableTime: true,
        dateFormat: "Y-m-d H:i",
        time_24hr: true,
        // minDate: "today", // 과거 날짜 선택 허용을 위해 주석 처리
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
        const matSelect = document.getElementById('order_material');
        const selectedOption = matSelect.options[matSelect.selectedIndex];
        // "PLA1 (Black)" 형태에서 재질 이름만 추출하거나, inventory 데이터에서 찾음
        const invId = matSelect.value;
        const inv = currentData.inventory.find(i => i.id === invId);

        const payload = {
            product_name: document.getElementById('order_product').value,
            target_quantity: parseFloat(document.getElementById('order_target').value),
            initial_stock: parseFloat(document.getElementById('order_initial_stock').value || 0),
            material: inv ? inv.material : '',
            color: document.getElementById('order_color').value,
            batch: inv ? inv.batch : '',
            unit_weight_g: parseFloat(document.getElementById('order_unit_weight').value || 0),
            deadline: document.getElementById('order_deadline').value
        };
        const res = await postData('/api/v1/orders', payload);
        if (res.ok) { await fetchData(); e.target.reset(); }
    });

    document.getElementById('order-edit-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const id = document.getElementById('edit_order_id').value;
        const matSelect = document.getElementById('edit_order_material');
        const invId = matSelect.value;
        const inv = currentData.inventory.find(i => i.id === invId);

        const payload = {
            product_name: document.getElementById('edit_order_product').value,
            target_quantity: parseFloat(document.getElementById('edit_order_target').value),
            initial_stock: parseFloat(document.getElementById('edit_order_initial_stock').value || 0),
            material: inv ? inv.material : matSelect.value,
            color: document.getElementById('edit_order_color').value,
            batch: inv ? inv.batch : '',
            unit_weight_g: parseFloat(document.getElementById('edit_order_unit_weight').value),
            deadline: document.getElementById('edit_order_deadline').value
        };
        const res = await postData(`/api/v1/orders/${id}`, payload, 'PATCH');
        if (res.ok) { closeOrderEdit(); await fetchData(); }
    });

    // 2. 재고 생성 및 수정
    document.getElementById('inventory-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const batchVal = document.getElementById('inv_batch').value;

        // 중복 배치 확인
        if (batchVal) {
            const isDuplicate = (currentData.inventory || []).some(i => i.batch === batchVal);
            if (isDuplicate) {
                if (!confirm(`'${batchVal}' 배치는 이미 등록된 이름입니다. 동일한 배치를 계속 추가하시겠습니까?`)) {
                    return; // 사용자가 '아니오'를 선택하면 중단
                }
            }
        }

        const payload = {
            material: document.getElementById('material').value, color: document.getElementById('color').value,
            batch: batchVal,
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
            batch: document.getElementById('edit_inv_batch').value,
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
        const plannedQty = parseFloat(document.getElementById('planned_qty').value);
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
        console.log("Fetching fresh data from server...");
        const response = await fetch('/api/v1/data');
        currentData = await response.json();
        renderOrders(currentData.orders || [], currentData.schedules || []);
        renderSchedules(currentData.schedules || [], currentData.orders || [], currentData.printers || []);
        renderDefectRecoveryAlerts(currentData.orders || [], currentData.schedules || []);
        renderDashboard(currentData);
        renderInventory(currentData.inventory || [], currentData.orders || []);
        renderPrinters(currentData.printers || []);
        updateSelects(currentData.orders, currentData.printers, currentData.inventory);
        renderTimeline(currentData);
        renderReport(currentData);
    } catch (error) { console.error('Error:', error); }
}

function initOrderFormEvents() {
    const productInput = document.getElementById('order_product');
    const materialSelect = document.getElementById('order_material');
    const colorSelect = document.getElementById('order_color');
    const weightInput = document.getElementById('order_unit_weight');

    if (!productInput || !materialSelect) return;

    // 1. 제품명 입력 시 이전 무게 추천
    productInput.addEventListener('blur', () => {
        const val = productInput.value.trim();
        if (!val) return;

        const pastOrder = (currentData.orders || []).find(o => o.product_name === val);
        if (pastOrder && pastOrder.unit_weight_g) {
            if (confirm(`'${val}' 제품의 기존 기록된 개당 무게는 ${pastOrder.unit_weight_g}g 입니다. 이 무게를 사용하시겠습니까?`)) {
                weightInput.value = pastOrder.unit_weight_g;
            }
        }
    });

    // 2. 재질 선택 시 색상 자동 선택 및 안내
    materialSelect.addEventListener('change', () => {
        const selectedId = materialSelect.value;
        const inv = (currentData.inventory || []).find(i => i.id === selectedId);
        if (inv) {
            colorSelect.value = inv.color;
            alert(`선택하신 [${inv.material} / ${inv.color}]의 현재 시스템 잔량은 ${inv.remaining_weight_g}g 입니다.`);
        }
    });
}

// --- Render Logic ---

function renderReport(data) {
    const filamentBody = document.getElementById('report-filament-body');
    const printerBody = document.getElementById('report-printer-body');
    const productRanking = document.getElementById('report-product-ranking');

    if (!filamentBody || !printerBody || !productRanking) return;

    filamentBody.innerHTML = '';
    printerBody.innerHTML = '';
    productRanking.innerHTML = '';

    // 1. 재질별 총 소모량
    const filamentUsage = {};
    data.schedules.forEach(s => {
        if (s.actual_quantity > 0) {
            const order = data.orders.find(o => o.id === s.order_id);
            if (order) {
                const key = `${order.material} / ${order.color}`;
                const weight = (parseFloat(s.actual_quantity) * parseFloat(order.unit_weight_g || 0));
                filamentUsage[key] = (filamentUsage[key] || 0) + weight;
            }
        }
    });

    Object.entries(filamentUsage).sort((a, b) => b[1] - a[1]).forEach(([key, weight]) => {
        filamentBody.innerHTML += `<tr><td>${key}</td><td>${weight.toLocaleString(undefined, { minimumFractionDigits: 1, maximumFractionDigits: 1 })}g</td></tr>`;
    });

    // 2. 프린터별 통계
    const printerStats = {};
    data.schedules.forEach(s => {
        const p = data.printers.find(pr => pr.id === s.printer_id) || { model: 'Unknown', asset_id: '-' };
        const key = `${p.model} (${p.asset_id})`;
        if (!printerStats[key]) printerStats[key] = { jobs: 0, total_qty: 0 };
        if (s.actual_quantity > 0) {
            printerStats[key].jobs += 1;
            printerStats[key].total_qty += parseFloat(s.actual_quantity);
        }
    });

    Object.entries(printerStats).sort((a, b) => b[1].total_qty - a[1].total_qty).forEach(([key, stats]) => {
        printerBody.innerHTML += `<tr><td>${key}</td><td>${stats.jobs}건</td><td>${stats.total_qty.toLocaleString()}개</td></tr>`;
    });

    // 3. 제품별 랭킹
    const productStats = {};
    data.orders.forEach(o => {
        const actual = data.schedules.filter(s => s.order_id === o.id).reduce((sum, s) => sum + parseFloat(s.actual_quantity), 0);
        productStats[o.product_name] = (productStats[o.product_name] || 0) + actual;
    });

    Object.entries(productStats).sort((a, b) => b[1] - a[1]).slice(0, 5).forEach(([name, qty]) => {
        productRanking.innerHTML += `
            <article>
                <h4>${name}</h4>
                <p class="data-value" style="font-size: 1.5em;">${qty.toLocaleString()}개</p>
            </article>
        `;
    });
}

function renderOrders(orders, schedules) {
    const container = document.getElementById('order-list');
    container.innerHTML = '';
    orders.forEach(order => {
        const relatedSchedules = schedules.filter(s => s.order_id === order.id);
        const actualSum = relatedSchedules.reduce((sum, s) => sum + (parseFloat(s.actual_quantity) || 0), 0);
        const initialStock = parseFloat(order.initial_stock || 0);
        const totalSecured = initialStock + actualSum;
        const progress = order.target_quantity ? Math.min(100, Math.floor((totalSecured / order.target_quantity) * 100)) : 0;
        const remaining = order.target_quantity - totalSecured;
        const totalUsage = (order.unit_weight_g || 0) * (order.target_quantity || 0);
        const remainingUsage = (order.unit_weight_g || 0) * (Math.max(0, remaining));

        const card = document.createElement('div');
        card.className = 'order-card';

        let imageHtml = `<div class="order-image-placeholder" onclick="document.getElementById('file-input-${order.id}').click()">사진 추가</div>`;
        if (order.image_url) {
            imageHtml = `<img src="${order.image_url}" class="order-image" onclick="document.getElementById('file-input-${order.id}').click()">`;
        }

        card.innerHTML = `
            <div class="order-header">
                <h3>${order.product_name}</h3>
                <div>
                    <button onclick="openOrderEdit('${order.id}')" class="btn-sm" style="background:#ffc107; color:#333;">수정</button>
                    <span class="badge ${order.status}">${order.status}</span>
                </div>
            </div>
            ${imageHtml}
            <input type="file" id="file-input-${order.id}" style="display:none" onchange="uploadPhoto('${order.id}', 'order', this)">
            <div class="progress-bar-bg">
                <div class="progress-bar-fill" style="width: ${progress}%"></div>
            </div>
            <div class="order-stats">
                <span>목표: ${order.target_quantity}</span>
                <span>확보: <strong>${totalSecured}</strong></span>
                <span>잔여: <span style="color: ${remaining > 0 ? 'red' : 'green'}">${Math.max(0, remaining)}</span></span>
            </div>
            <div style="font-size: 0.85em; color: #555; margin-top: 5px;">
                사용: ${order.batch ? `<span style="background:#6f42c1; color:white; padding:1px 4px; border-radius:3px; font-size:0.9em; margin-right:3px;">${order.batch}</span>` : ''}${order.material}/${order.color} (${order.unit_weight_g}g/ea)<br>
                총 소모(예정): <strong>${totalUsage.toLocaleString()}g</strong> 
                ${remainingUsage > 0 ? ` / <span style="color:red;">잔여 필요: ${remainingUsage.toLocaleString()}g</span>` : ' (완료)'}
            </div>
            <div style="text-align: right; margin-top: 5px; font-size: 0.85em; color: #666;">
                마감: ${order.deadline || '없음'} 
                <button onclick="deleteOrder('${order.id}')" class="btn-sm btn-danger">삭제</button>
            </div>
        `;
        container.appendChild(card);
    });
}

async function uploadPhoto(targetId, targetType, input) {
    if (!input.files || !input.files[0]) return;

    const formData = new FormData();
    formData.append('file', input.files[0]);
    formData.append('target_id', targetId);
    formData.append('target_type', targetType);

    try {
        const res = await fetch('/api/v1/upload', {
            method: 'POST',
            body: formData
        });
        if (res.ok) {
            await fetchData();
        } else {
            const err = await res.json();
            alert('업로드 실패: ' + err.message);
        }
    } catch (err) {
        console.error('Upload error', err);
        alert('업로드 중 오류가 발생했습니다.');
    }
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
    if (document.getElementById('edit_order_batch')) {
        document.getElementById('edit_order_batch').value = order.batch || '';
    }
    document.getElementById('edit_order_unit_weight').value = order.unit_weight_g || 0;
    document.getElementById('edit_order_deadline').value = order.deadline || '';
    document.getElementById('order-edit-overlay').style.display = 'block';
}
function closeOrderEdit() { document.getElementById('order-edit-overlay').style.display = 'none'; }

function updateFilamentSelects(inventory) {
    const materialSelects = [document.getElementById('order_material'), document.getElementById('edit_order_material')];
    const colorSelects = [document.getElementById('order_color'), document.getElementById('edit_order_color')];

    // 재질 선택 시 "[배치] 재질명 (색상)" 형식으로 표시
    const materialOptions = '<option value="">-- 재질 선택 --</option>' +
        inventory.map(i => `<option value="${i.id}">${i.batch ? `[${i.batch}] ` : ''}${i.material} (${i.color})</option>`).join('');

    const colors = [...new Set(inventory.map(i => i.color))].sort();
    const colorOptions = '<option value="">-- 색상 선택 --</option>' +
        colors.map(c => `<option value="${c}">${c}</option>`).join('');

    materialSelects.forEach(sel => { if (!sel) return; const cur = sel.value; sel.innerHTML = materialOptions; if (cur) sel.value = cur; });
    colorSelects.forEach(sel => { if (!sel) return; const cur = sel.value; sel.innerHTML = colorOptions; if (cur) sel.value = cur; });
}

function renderSchedules(schedules, orders, printers) {
    const tbody = document.getElementById('schedule-body');
    if (!tbody) return;
    tbody.innerHTML = '';
    const sortedSchedules = schedules.slice().sort((a, b) => new Date(a.start_time) - new Date(b.start_time));
    let lastOrderId = null;
    sortedSchedules.forEach(s => {
        // 주문이 삭제되었을 경우를 대비한 기본값 처리
        const order = orders.find(o => o.id === s.order_id) || { product_name: '삭제된 주문', material: '?', color: '?' };
        const printer = printers.find(p => p.id === s.printer_id) || { model: '미지정', asset_id: '-' };

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
                ${s.defect_quantity ? `
                    <div class="tooltip" style="color:var(--danger-color); font-size:0.9em; margin-top:2px;">
                        불량: ${s.defect_quantity}
                        <span class="tooltip-text">${(s.defect_reasons || []).join('\n') || '사유 미기재'}</span>
                    </div>` : ''}
            </td>
            <td>
                <div style="display: flex; gap: 5px; justify-content: center; align-items: center; flex-wrap: wrap;">
                    <button onclick="updateRun('${s.id}', prompt('추가 성공 수량 입력 (누적됨):', '0'))" class="btn-sm">실적입력</button>
                    <button onclick="updateRunDefect('${s.id}')" class="btn-sm" style="background:var(--danger-color);">불량입력</button>
                    <button onclick="deleteSchedule('${s.id}')" class="btn-sm btn-danger">삭제</button>
                </div>
            </td>
            <td><small style="color:#999;">${s.id}</small></td>
        `;
        tbody.appendChild(tr);
        lastOrderId = s.order_id;
    });

    // 최근 생산 품목 리스트 업데이트
    renderRecentProducts(orders);
}

function renderRecentProducts(orders) {
    const container = document.getElementById('recent-products-list');
    if (!container) return;

    // 중복 제거 및 최신순 정렬
    const recent = [];
    const seen = new Set();

    // 주문 리스트를 뒤에서부터 훑으며 고유한 제품명 5개 추출
    [...orders].reverse().forEach(o => {
        if (!seen.has(o.product_name)) {
            seen.add(o.product_name);
            recent.push(o);
        }
    });

    container.innerHTML = recent.slice(0, 5).map(o => `
        <button type="button" class="btn-sm" style="margin-right:5px; margin-bottom:5px; background:#f8f9fa; color:#333; border:1px solid #ddd; border-radius:12px; padding: 2px 10px;" 
            onclick="quickFillOrder('${o.product_name}', ${o.unit_weight_g})">
            ${o.product_name} (${o.unit_weight_g}g)
        </button>
    `).join('');
}

function quickFillOrder(name, weight) {
    document.getElementById('order_product').value = name;
    document.getElementById('order_unit_weight').value = weight;
    // 시각적 피드백
    const input = document.getElementById('order_product');
    input.style.backgroundColor = '#fff3cd';
    setTimeout(() => { input.style.backgroundColor = ''; }, 500);
}

async function updateRunDefect(id) {
    const qty = prompt('불량 수량 입력 (누적됨):', '0');
    if (qty === null || qty === "" || isNaN(qty)) return;

    const reason = prompt('불량 사유 입력:', '노즐 막힘');
    if (reason === null) return;

    const res = await postData(`/api/v1/schedules/${id}`, {
        defect_quantity: parseFloat(qty),
        defect_reason: reason
    }, 'PATCH');

    if (res.ok) {
        fetchData();
    }
}

/**
 * 불량으로 인한 수량 부족을 확인하고 경고 알림을 렌더링함
 */
function renderDefectRecoveryAlerts(orders, schedules) {
    console.log("Checking for defect recovery shortages...");
    const container = document.getElementById('defect-recovery-container');
    if (!container) {
        console.error("Alert container not found!");
        return;
    }
    container.innerHTML = '';

    orders.forEach(order => {
        // 해당 주문의 실적 합계
        const actualTotal = schedules
            .filter(s => s.order_id === order.id)
            .reduce((sum, s) => sum + (parseFloat(s.actual_quantity) || 0), 0);

        // 해당 주문의 미래 계획 합계 (status가 pending인 것)
        const plannedFutureTotal = schedules
            .filter(s => s.order_id === order.id && s.status === 'pending')
            .reduce((sum, s) => sum + (parseFloat(s.planned_quantity) || 0), 0);

        // 부족 수량 계산 = 목표 - (기존 재고 + 실적 + 미래 계획)
        const target = parseFloat(order.target_quantity) || 0;
        const initial = parseFloat(order.initial_stock) || 0;
        const shortage = target - (initial + actualTotal + plannedFutureTotal);

        // 불량이 발생했었고(중요), 부족분이 있다면 알림 표시
        const ordersWithDefects = schedules.some(s => s.order_id === order.id && (parseFloat(s.defect_quantity) || 0) > 0);

        if (shortage > 0 && ordersWithDefects) {
            console.log(`Shortage found for ${order.product_name}: ${shortage} units`);
            const alertDiv = document.createElement('div');
            alertDiv.className = 'recovery-alert';
            alertDiv.innerHTML = `
                <div class="recovery-info">
                    <div class="recovery-title">⚠️ 생산 수량 부족 알림: ${order.product_name}</div>
                    <div class="recovery-msg">불량 발생 등으로 인해 목표 대비 <strong>${shortage}개</strong>가 부족합니다. (재고/계획 포함)</div>
                    <div style="color: var(--danger-color); font-weight: bold; margin-top: 5px;">※ 필라멘트 잔량을 다시 확인하세요!</div>
                </div>
                <button class="btn-recovery" onclick="quickScheduleReplacement('${order.id}', ${shortage})">복구 일정 추가</button>
            `;
            container.appendChild(alertDiv);
        }
    });
}

/**
 * 알림 클릭 시 입력 폼을 부족 수량으로 자동 세팅
 */
function quickScheduleReplacement(orderId, qty) {
    const orderSelect = document.getElementById('schedule_order_id');
    const qtyInput = document.getElementById('planned_qty');
    const form = document.getElementById('schedule-form');

    if (orderSelect && qtyInput) {
        orderSelect.value = orderId;
        qtyInput.value = qty;

        // 시각적 피드백: 입력창 강조
        qtyInput.style.boxShadow = '0 0 10px var(--warning-color)';
        setTimeout(() => { qtyInput.style.boxShadow = ''; }, 2000);

        // 폼으로 스크롤
        form.scrollIntoView({ behavior: 'smooth', block: 'center' });
    }
}

async function updateRun(id, qtyVal) {
    if (qtyVal === null || qtyVal === "" || isNaN(qtyVal)) return;
    const res = await postData(`/api/v1/schedules/${id}`, { actual_quantity: parseFloat(qtyVal), status: 'completed' }, 'PATCH');

    if (res.ok) {
        const result = await res.json();
        // 주문이 방금 완료되었는지 확인
        if (result.order_info && result.order_info.status === 'completed') {
            await handleOrderCompletion(result.order_info);
        }
    }
    fetchData();
}

async function handleOrderCompletion(order) {
    const useConfirm = confirm(`[축하합니다!] '${order.product_name}' 주문의 모든 생산이 완료되었습니다.\n\n사용한 필라멘트(${order.material}/${order.color})들의 현재 잔량을 실측하여 기록하시겠습니까?\n(취소를 누르면 기록 없이 종료됩니다.)`);

    if (!useConfirm) return;

    // 해당 재질/색상의 인벤토리 목록 가져오기 (유연한 검색)
    const data = currentData;
    const relevantInv = (data.inventory || []).filter(i => {
        const colorMatch = i.color === order.color;
        const materialMatch = i.material.includes(order.material) || order.material.includes(i.material);
        return colorMatch && materialMatch;
    });

    if (relevantInv.length === 0) {
        alert("등록된 해당 필라멘트 재고 정보가 없습니다.");
        return;
    }

    alert(`총 ${relevantInv.length}개의 관련 필라멘트가 검색되었습니다.\n지금부터 하나씩 무게를 입력받습니다. 변경을 원치 않는 항목은 빈칸으로 두거나 [취소]를 누르세요.`);

    for (const inv of relevantInv) {
        const currentEst = (inv.remaining_weight_g || 0);
        const weight = prompt(`[${inv.material} / ${inv.color}] (ID: ${inv.id})\n현재 기록된 잔량: ${currentEst}g\n\n새로운 측정 무게(g)를 입력하세요:`, "");

        // 빈칸이 아니고 숫자인 경우에만 업데이트
        if (weight !== null && weight !== "" && !isNaN(weight)) {
            await postData(`/api/v1/inventory/${inv.id}`, { remaining_weight_g: parseFloat(weight) }, 'PATCH');
        }
    }

    alert("모든 실측 잔량 기록 절차가 완료되었습니다.");
    fetchData();
}

async function postData(url, data, method = 'POST') { return fetch(url, { method: method, headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(data) }); }
async function deleteOrder(id) {
    if (confirm('주문을 삭제하시겠습니까?')) {
        const deleteSchedules = confirm('이 주문과 연결된 모든 생산 계획(일정)도 같이 삭제하시겠습니까?\n\n[확인] : 계획도 삭제\n[취소] : 주문만 삭제 (계획 유지)');
        await fetch(`/api/v1/orders/${id}?delete_schedules=${deleteSchedules}`, { method: 'DELETE' });
        fetchData();
    }
}
async function deleteSchedule(id) { if (confirm('계획 삭제?')) { await fetch(`/api/v1/schedules/${id}`, { method: 'DELETE' }); fetchData(); } }
async function deletePrinter(id) { if (confirm('프린터 삭제?')) { await fetch(`/api/v1/printers/${id}`, { method: 'DELETE' }); fetchData(); } }
async function deleteInventory(id) { if (confirm('재고 삭제?')) { await fetch(`/api/v1/inventory/${id}`, { method: 'DELETE' }); fetchData(); } }

function updateSelects(orders, printers, inventory) {
    const os = document.getElementById('schedule_order_id');
    const ps = document.getElementById('printer_id');
    const om = document.getElementById('order_material');
    const oc = document.getElementById('order_color');
    const em = document.getElementById('edit_order_material');
    const ec = document.getElementById('edit_order_color');

    // 1. 작업 할당용 주문 선택
    if (os && orders) {
        const curValue = os.value;
        os.innerHTML = '<option value="">-- 주문 선택 --</option>' +
            orders.filter(o => o.status !== 'completed')
                .map(o => `<option value="${o.id}">${o.product_name}</option>`).join('');
        os.value = curValue;
    }

    // 2. 프린터 선택
    if (ps && printers) {
        const curValue = ps.value;
        ps.innerHTML = '<option value="">-- 프린터 선택 --</option>' +
            printers.map(p => `<option value="${p.id}">${p.model} (${p.asset_id})</option>`).join('');
        ps.value = curValue;
    }

    // 3. 재고 기반 재질/색상 선택 (주문 등록용)
    if (inventory) {
        const materialOptions = '<option value="">-- 재질 선택 --</option>' +
            inventory.map(i => `<option value="${i.id}">${i.batch ? `[${i.batch}] ` : ''}${i.material} (${i.color})</option>`).join('');

        const colors = [...new Set(inventory.map(i => i.color))].sort();
        const colorOptions = '<option value="">-- 색상 선택 --</option>' +
            colors.map(c => `<option value="${c}">${c}</option>`).join('');

        if (om) { const cur = om.value; om.innerHTML = materialOptions; om.value = cur; }
        if (em) { const cur = em.value; em.innerHTML = materialOptions; em.value = cur; }
        if (oc) { const cur = oc.value; oc.innerHTML = colorOptions; oc.value = cur; }
        if (ec) { const cur = ec.value; ec.innerHTML = colorOptions; ec.value = cur; }
    }
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

    // 1. 재질/색상별 데이터 집계 (모든 배치 통합)
    const stats = {};

    // 패밀리 키 생성 함수 (재질명과 색상만으로 그룹화)
    const getFamilyKey = (item) => {
        const m = item.material.toLowerCase();
        const c = item.color.toLowerCase();

        // 주문에서 사용하는 대표 재질명 찾기
        const matchedOrder = orders.find(o => {
            const om = o.material.toLowerCase();
            return (m.includes(om) || om.includes(m)) && o.color.toLowerCase() === c;
        });

        const baseMat = matchedOrder ? matchedOrder.material.toLowerCase() : m.replace(/[0-9\s]/g, '').toLowerCase();
        return `${baseMat}_${c}`;
    };

    const getStats = (item) => {
        const key = getFamilyKey(item);
        if (!stats[key]) stats[key] = { stock: 0, estimatedCurrentStock: 0, future: 0, batches: new Set() };
        return stats[key];
    };

    // 재고 기반으로 현재 추정 잔량 계산 및 '전체 합산'
    inv.forEach(i => {
        const s = getStats(i);
        if (i.batch) s.batches.add(i.batch);
        else s.batches.add(i.material); // 배치가 없으면 재질명이라도 넣어서 식별

        const lastUpdate = i.updated_at ? new Date(i.updated_at) : new Date(0);

        // 해당 재고(i)에서 차감되어야 할 무게 계산 (배치 및 재질/색상 일치 여부 확인)
        // Helper function call
        const consumedWeight = calculateConsumedWeight(i, currentData.schedules || [], currentData.orders || []);

        const itemEstimatedStock = parseFloat(i.remaining_weight_g || 0) - consumedWeight;

        s.estimatedCurrentStock += itemEstimatedStock;
    });

    // 향후 필요량 계산 (재질 그룹별로 단 한 번만 계산)
    orders.filter(o => o.status !== 'completed').forEach(o => {
        const key = `${o.material.toLowerCase()}_${o.color.toLowerCase()}`;
        // 배치가 지정된 주문은 해당 배치 그룹에만 영향을 줘야 하나, 
        // 현재 UI는 "재질_색상" 단위로 통합 뷰를 제공하므로, 전체 필요량에 합산함.
        // (상세 뷰에서는 배치별 구분이 필요할 수 있음)
        if (stats[key]) {
            const actualSum = (currentData.schedules || []).filter(sch => sch.order_id === o.id).reduce((sum, sch) => sum + (parseFloat(sch.actual_quantity) || 0), 0);
            const remainingQty = Math.max(0, o.target_quantity - o.initial_stock - actualSum);
            stats[key].future += remainingQty * (o.unit_weight_g || 0);
        }
    });

    // 2. 렌더링
    const sortedInv = inv.slice().sort((a, b) => {
        const matCompare = a.material.localeCompare(b.material, undefined, { numeric: true, sensitivity: 'base' });
        if (matCompare !== 0) return matCompare;
        return a.color.localeCompare(b.color, undefined, { numeric: true, sensitivity: 'base' });
    });

    sortedInv.forEach(i => {
        const s = getStats(i);

        // 개별 아이템 표시용 잔량 계산 (위와 동일한 로직 반복 - 함수화 가능하지만 일단 인라인 처리)
        const lastUpdate = i.updated_at ? new Date(i.updated_at) : new Date(0);
        // Helper function call
        const consumedWeight = calculateConsumedWeight(i, currentData.schedules || [], currentData.orders || []);

        const itemEstimatedStock = parseFloat(i.remaining_weight_g || 0) - consumedWeight;

        const isShort = s.estimatedCurrentStock < s.future;
        const tr = document.createElement('tr');
        if (isShort) tr.className = 'conflict-row';

        // [복원된 로직] 재고 상태 텍스트 생성 (v02 스타일)
        let statusHtml = '';
        if (s.future > 0) {
            const batchStr = Array.from(s.batches).sort().join('+');
            const totalStock = s.estimatedCurrentStock.toFixed(1);
            const needed = s.future.toFixed(1);

            if (isShort) {
                statusHtml = `<br><small style="color:red; font-weight:bold;">[부족] 통합(${batchStr}): ${totalStock}g / 필요: ${needed}g</small>`;
            } else {
                const surplus = (s.estimatedCurrentStock - s.future).toFixed(1);
                statusHtml = `<br><small style="color:green; font-weight:bold;">[충분] 통합(${batchStr}): ${totalStock}g / 여유: ${surplus}g</small>`;
            }
        } else {
            // 필요량이 없을 때도(주문 완료 등) 현재 개별 항목의 소모 상태를 표시
            // (v02에는 없었을 수 있으나, v03 맥락에 맞춰 개별 소모량 표시 추가 고려 가능. 
            //  일단 사용자가 요청한 건 "녹색 글씨로 소모된 필라멘트 량을 고려해서 수학적 접근을 표시" 한 것임.)
            if (consumedWeight > 0) {
                statusHtml = `<br><small style="color:blue;">(실측 후 소모: -${consumedWeight.toFixed(1)}g)</small>`;
            }
        }

        const batchBadge = i.batch ? `<span style="background:#6f42c1; color:white; padding:2px 6px; border-radius:4px; font-size:0.8em; margin-right:5px;">${i.batch}</span>` : '';

        tr.innerHTML = `
            <td>${batchBadge}${i.material}</td>
            <td><span style="${getColorStyle(i.color)}">${i.color}</span></td>
            <td>
                <span title="등록 무게: ${i.remaining_weight_g}g / 실측 후 소모: ${consumedWeight.toFixed(1)}g">${itemEstimatedStock.toFixed(1)}g</span>
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
        const logs = p.maintenance_logs || [];
        const latestLog = logs.length > 0 ? logs[logs.length - 1].date : '-';

        tr.innerHTML = `
            <td>${p.model}</td>
            <td>${p.asset_id}</td>
            <td>${p.purchase_date}</td>
            <td>
                <div style="font-size:0.85em; color:#666;">최근점검: ${latestLog}</div>
                <button onclick="addMaintenance('${p.id}')" class="btn-sm" style="background:#17a2b8; margin-bottom:5px;">점검기록</button><br>
                <button onclick="openPrinterEdit('${p.id}')" class="btn-sm">수정</button>
                <button onclick="deletePrinter('${p.id}')" class="btn-sm btn-danger">삭제</button>
            </td>`;
        tb.appendChild(tr);
    });
}

async function addMaintenance(id) {
    const desc = prompt("유지보수 내용을 입력하세요 (예: 노즐 교체, 베드 레벨링):", "정기 점검");
    if (desc === null) return;

    const date = new Date().toISOString().split('T')[0];
    const res = await postData(`/api/v1/printers/${id}/maintenance`, { description: desc, date: date });
    if (res.ok) {
        await fetchData();
    }
}

function renderDashboard(data) {
    const totalTarget = (data.orders || []).reduce((sum, o) => sum + parseFloat(o.target_quantity), 0);
    const totalSecured = (data.orders || []).reduce((sum, o) => sum + parseFloat(o.initial_stock) + parseFloat(o.completed_quantity), 0);
    const percent = totalTarget ? Math.floor((totalSecured / totalTarget) * 100) : 0;
    document.querySelector('#stat-progress .data-value').textContent = `${percent}%`;
    document.querySelector('#stat-printers .data-value').textContent = (data.printers || []).length;
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
    document.getElementById('edit_inv_batch').value = item.batch || '';
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

/**
 * Calculates the total weight consumed for a specific inventory item based on production schedules.
 * @param {Object} item - The inventory item (must have id, material, color, updated_at).
 * @param {Array} schedules - List of all schedule/task objects.
 * @param {Array} orders - List of all order objects.
 * @returns {number} - The calculated consumed weight in grams.
 */
function calculateConsumedWeight(item, schedules, orders) {
    const lastUpdate = item.updated_at ? new Date(item.updated_at) : new Date(0);

    return schedules.reduce((sum, sch) => {
        const order = orders.find(o => o.id === sch.order_id);
        if (!order) return sum;

        // 1. 색상 일치 확인
        if (order.color.toLowerCase() !== item.color.toLowerCase()) return sum;

        // 2. 재질 포함 여부 확인
        const m1 = item.material.toLowerCase();
        const m2 = order.material.toLowerCase();
        if (!(m1.includes(m2) || m2.includes(m1))) return sum;

        // 3. 배치 일치 확인
        if (order.batch && order.batch !== item.batch) return sum;

        // 4. 시간 확인 (생산 시작 시간 기준)
        // db에 저장된 start_time은 "YYYY-MM-DDTHH:mm" 형식임
        const actionTime = sch.start_time ? new Date(sch.start_time) : new Date(0);
        if (actionTime <= lastUpdate) return sum;

        // 5. 무게 누적 (성공 실적 + 불량 실적 모두 합산 소모)
        const actualQty = parseFloat(sch.actual_quantity) || 0;
        const defectQty = parseFloat(sch.defect_quantity) || 0;
        return sum + ((actualQty + defectQty) * (parseFloat(order.unit_weight_g) || 0));
    }, 0);
}
