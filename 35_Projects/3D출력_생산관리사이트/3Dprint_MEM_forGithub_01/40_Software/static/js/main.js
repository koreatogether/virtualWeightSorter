document.addEventListener('DOMContentLoaded', () => {
    initOrderFormEvents();
    // ?뚮쭏 ?좉?
    document.getElementById('theme-toggle').addEventListener('click', () => {
        document.body.classList.toggle('dark-mode');
    });

    // --- Flatpickr 珥덇린??---
    let datePicker = flatpickr("#start_date_picker", {
        enableTime: true,
        dateFormat: "Y-m-d H:i",
        time_24hr: true,
        minDate: "today",
        disable: [] // 珥덇린??鍮꾪솢???좎쭨 ?놁쓬
    });

    // ?꾨┛???좏깮 ??諛붿걶 ?좎쭨 媛?몄삤湲?
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
                // Flatpickr??鍮꾪솢???좎쭨 ?ㅼ젙
                datePicker.set('disable', busyDates);
            }
        } catch (err) {
            console.error('Failed to fetch busy dates', err);
        }
    }

    // ?먮룞 怨꾪쉷 紐⑤뱶 ?좉?
    const autoModeCheck = document.getElementById('auto_schedule_mode');
    const scheduleBtn = document.getElementById('schedule-btn');

    autoModeCheck.addEventListener('change', (e) => {
        const isAuto = e.target.checked;
        if (isAuto) {
            scheduleBtn.textContent = '?먮룞 怨꾪쉷 ?앹꽦 (?쇨큵 ?깅줉)';
            // ?먮룞 紐⑤뱶???좎쭨留??꾩슂
            datePicker.set('enableTime', false);
            datePicker.set('dateFormat', "Y-m-d");
        } else {
            scheduleBtn.textContent = '?묒뾽 ?좊떦';
            // ?섎룞 紐⑤뱶???쒓컙源뚯? ?꾩슂
            datePicker.set('enableTime', true);
            datePicker.set('dateFormat', "Y-m-d H:i");
        }
    });

    // 1. 二쇰Ц ?앹꽦 諛??섏젙
    document.getElementById('order-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const matSelect = document.getElementById('order_material');
        const selectedOption = matSelect.options[matSelect.selectedIndex];
        // "PLA1 (Black)" ?뺥깭?먯꽌 ?ъ쭏 ?대쫫留?異붿텧?섍굅?? inventory ?곗씠?곗뿉??李얠쓬
        const invId = matSelect.value;
        const inv = currentData.inventory.find(i => i.id === invId);

        const payload = {
            product_name: document.getElementById('order_product').value,
            target_quantity: parseFloat(document.getElementById('order_target').value),
            initial_stock: parseFloat(document.getElementById('order_initial_stock').value || 0),
            material: inv ? inv.material : '', 
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
            target_quantity: parseFloat(document.getElementById('edit_order_target').value),
            initial_stock: parseFloat(document.getElementById('edit_order_initial_stock').value || 0),
            material: document.getElementById('edit_order_material').value,
            color: document.getElementById('edit_order_color').value,
            unit_weight_g: parseFloat(document.getElementById('edit_order_unit_weight').value),
            deadline: document.getElementById('edit_order_deadline').value
        };
        const res = await postData(`/api/v1/orders/${id}`, payload, 'PATCH');
        if (res.ok) { closeOrderEdit(); await fetchData(); }
    });

    // 2. ?ш퀬 ?앹꽦 諛??섏젙
    document.getElementById('inventory-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const batchVal = document.getElementById('inv_batch').value;
        
        // 以묐났 諛곗튂 ?뺤씤
        if (batchVal) {
            const isDuplicate = (currentData.inventory || []).some(i => i.batch === batchVal);
            if (isDuplicate) {
                if (!confirm(`'${batchVal}' 諛곗튂???대? ?깅줉???대쫫?낅땲?? ?숈씪??諛곗튂瑜?怨꾩냽 異붽??섏떆寃좎뒿?덇퉴?`)) {
                    return; // ?ъ슜?먭? '?꾨땲??瑜??좏깮?섎㈃ 以묐떒
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

    // 3. ?꾨┛???앹꽦 諛??섏젙
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

    // 4. ?앹궛 ?묒뾽 ?좊떦 (Schedule)
    document.getElementById('schedule-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        const isAuto = document.getElementById('auto_schedule_mode').checked;
        const orderId = document.getElementById('schedule_order_id').value;
        const printerId = document.getElementById('printer_id').value;
        const plannedQty = parseFloat(document.getElementById('planned_qty').value);
        const dateValue = document.getElementById('start_date_picker').value;

        if (!dateValue) return alert("?좎쭨瑜??좏깮?댁＜?몄슂.");

        if (isAuto) {
            // dateValue: YYYY-MM-DD
            const payload = { order_id: orderId, printer_id: printerId, daily_qty: plannedQty, start_date: dateValue };
            const res = await postData('/api/v1/schedules/auto', payload);
            if (res.ok) { 
                const data = await res.json();
                let msg = `${data.count}?쇱튂 ?앹궛 怨꾪쉷???먮룞 ?앹꽦?섏뿀?듬땲??`;
                if (data.conflicts && data.conflicts.length > 0) msg += `\n[寃쎄퀬] ?좎쭨 以묐났 媛먯?: ${data.conflicts.join(', ')}`;
                alert(msg); 
                // ?대떦 ?꾨┛?곗쓽 諛붿걶 ?좎쭨 媛깆떊
                await updateBusyDates(printerId);
                await fetchData(); 
                e.target.reset();
            } else { const err = await res.json(); alert('?ㅻ쪟: ' + err.message); }
        } else {
            // dateValue: YYYY-MM-DD HH:mm (T 異붽? ?꾩슂)
            const isoTime = dateValue.replace(' ', 'T');
            const payload = { order_id: orderId, printer_id: printerId, planned_quantity: plannedQty, start_time: isoTime };
            const res = await postData('/api/v1/schedules', payload);
            if (res.ok) { 
                const data = await res.json(); 
                if (data.conflict) alert("寃쎄퀬: 以묐났 ?덉빟 媛먯?!"); 
                await updateBusyDates(printerId);
                await fetchData(); 
                e.target.reset(); 
            }
        }
    });

    fetchData();
});

let currentData = null;
const API_MUTATION_HEADERS = { 'Content-Type': 'application/json', 'X-Requested-With': 'XMLHttpRequest' };

function initOrderFormEvents() {
    const productInput = document.getElementById('order_product');
    const materialSelect = document.getElementById('order_material');
    const colorSelect = document.getElementById('order_color');
    const weightInput = document.getElementById('order_unit_weight');

    if (!productInput || !materialSelect) return;

    // 1. ?쒗뭹紐??낅젰 ???댁쟾 臾닿쾶 異붿쿇
    productInput.addEventListener('blur', () => {
        const val = productInput.value.trim();
        if (!val) return;

        const pastOrder = (currentData.orders || []).find(o => o.product_name === val);
        if (pastOrder && pastOrder.unit_weight_g) {
            if (confirm(`'${val}' ?쒗뭹??湲곗〈 湲곕줉??媛쒕떦 臾닿쾶??${pastOrder.unit_weight_g}g ?낅땲?? ??臾닿쾶瑜??ъ슜?섏떆寃좎뒿?덇퉴?`)) {
                weightInput.value = pastOrder.unit_weight_g;
            }
        }
    });

    // 2. ?ъ쭏 ?좏깮 ???됱긽 ?먮룞 ?좏깮 諛??덈궡
    materialSelect.addEventListener('change', () => {
        const selectedId = materialSelect.value;
        const inv = (currentData.inventory || []).find(i => i.id === selectedId);
        if (inv) {
            colorSelect.value = inv.color;
            alert(`?좏깮?섏떊 [${inv.material} / ${inv.color}]???꾩옱 ?쒖뒪???붾웾? ${inv.remaining_weight_g}g ?낅땲??`);
        }
    });
}

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
        const actualSum = relatedSchedules.reduce((sum, s) => sum + (parseFloat(s.actual_quantity) || 0), 0);
        const initialStock = parseFloat(order.initial_stock || 0);
        const totalSecured = initialStock + actualSum;
        const progress = order.target_quantity ? Math.min(100, Math.floor((totalSecured / order.target_quantity) * 100)) : 0;
        const remaining = order.target_quantity - totalSecured;
        const totalUsage = (order.unit_weight_g || 0) * (order.target_quantity || 0);
        const remainingUsage = (order.unit_weight_g || 0) * (Math.max(0, remaining));

        const card = document.createElement('div');
        card.className = 'order-card';
        card.innerHTML = `
            <div class="order-header">
                <h3>${order.product_name}</h3>
                <div>
                    <button onclick="openOrderEdit('${order.id}')" class="btn-sm" style="background:#ffc107; color:#333;">?섏젙</button>
                    <span class="badge ${order.status}">${order.status}</span>
                </div>
            </div>
            <div class="progress-bar-bg">
                <div class="progress-bar-fill" style="width: ${progress}%"></div>
            </div>
            <div class="order-stats">
                <span>紐⑺몴: ${order.target_quantity}</span>
                <span>?뺣낫: <strong>${totalSecured}</strong></span>
                <span>?붿뿬: <span style="color: ${remaining > 0 ? 'red' : 'green'}">${Math.max(0, remaining)}</span></span>
            </div>
            <div style="font-size: 0.85em; color: #555; margin-top: 5px;">
                ?ъ슜: ${order.material}/${order.color} (${order.unit_weight_g}g/ea)<br>
                珥??뚮え(?덉젙): <strong>${totalUsage.toLocaleString()}g</strong> 
                ${remainingUsage > 0 ? ` / <span style="color:red;">?붿뿬 ?꾩슂: ${remainingUsage.toLocaleString()}g</span>` : ' (?꾨즺)'}
            </div>
            <div style="text-align: right; margin-top: 5px; font-size: 0.85em; color: #666;">
                留덇컧: ${order.deadline || '?놁쓬'} 
                <button onclick="deleteOrder('${order.id}')" class="btn-sm btn-danger">??젣</button>
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
    
    // ?ъ쭏 ?좏깮 ??"?ъ쭏紐?(?됱긽)" ?뺤떇?쇰줈 ?쒖떆?섍퀬, value??inventory??ID濡??ㅼ젙
    const materialOptions = '<option value="">-- ?ъ쭏 ?좏깮 --</option>' + 
        inventory.map(i => `<option value="${i.id}">${i.material} (${i.color})</option>`).join('');
    
    const colors = [...new Set(inventory.map(i => i.color))].sort();
    const colorOptions = '<option value="">-- ?됱긽 ?좏깮 --</option>' + 
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
        // 二쇰Ц????젣?섏뿀??寃쎌슦瑜??鍮꾪븳 湲곕낯媛?泥섎━
        const order = orders.find(o => o.id === s.order_id) || { product_name: '??젣??二쇰Ц', material: '?', color: '?' };
        const printer = printers.find(p => p.id === s.printer_id) || { model: '誘몄???, asset_id: '-' };

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
                <div style="color:var(--accent-color)">怨꾪쉷: ${s.planned_quantity}</div>
                <div style="font-weight:bold;">?ㅼ쟻: ${s.actual_quantity}</div>
            </td>
            <td>
                <div style="display: flex; gap: 5px; justify-content: center; align-items: center;">
                    <button onclick="updateRun('${s.id}', prompt('異붽? ?깃났 ?섎웾 ?낅젰 (?꾩쟻??:', '0'))" class="btn-sm">?ㅼ쟻?낅젰</button>
                    <button onclick="deleteSchedule('${s.id}')" class="btn-sm btn-danger">??젣</button>
                </div>
            </td>
            <td><small style="color:#999;">${s.id}</small></td>
        `;
        tbody.appendChild(tr);
        lastOrderId = s.order_id;
    });

    // 理쒓렐 ?앹궛 ?덈ぉ 由ъ뒪???낅뜲?댄듃
    renderRecentProducts(orders);
}

function renderRecentProducts(orders) {
    const container = document.getElementById('recent-products-list');
    if (!container) return;
    
    // 以묐났 ?쒓굅 諛?理쒖떊???뺣젹
    const recent = [];
    const seen = new Set();
    
    // 二쇰Ц 由ъ뒪?몃? ?ㅼ뿉?쒕????묒쑝硫?怨좎쑀???쒗뭹紐?5媛?異붿텧
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
    // ?쒓컖???쇰뱶諛?
    const input = document.getElementById('order_product');
    input.style.backgroundColor = '#fff3cd';
    setTimeout(() => { input.style.backgroundColor = ''; }, 500);
}

async function updateRun(id, qtyVal) {
    if (qtyVal === null) return;
    const res = await postData(`/api/v1/schedules/${id}`, { actual_quantity: parseFloat(qtyVal), status: 'completed' }, 'PATCH');
    
    if (res.ok) {
        const result = await res.json();
        // 二쇰Ц??諛⑷툑 ?꾨즺?섏뿀?붿? ?뺤씤
        if (result.order_info && result.order_info.status === 'completed') {
            await handleOrderCompletion(result.order_info);
        }
    }
    fetchData();
}

async function handleOrderCompletion(order) {
    const useConfirm = confirm(`[異뺥븯?⑸땲??] '${order.product_name}' 二쇰Ц??紐⑤뱺 ?앹궛???꾨즺?섏뿀?듬땲??\n\n?ъ슜???꾨씪硫섑듃(${order.material}/${order.color})?ㅼ쓽 ?꾩옱 ?붾웾???ㅼ륫?섏뿬 湲곕줉?섏떆寃좎뒿?덇퉴?\n(痍⑥냼瑜??꾨Ⅴ硫?湲곕줉 ?놁씠 醫낅즺?⑸땲??)`);
    
    if (!useConfirm) return;

    // ?대떦 ?ъ쭏/?됱긽???몃깽?좊━ 紐⑸줉 媛?몄삤湲?(?좎뿰??寃??
    const data = currentData; 
    const relevantInv = (data.inventory || []).filter(i => {
        const colorMatch = i.color === order.color;
        const materialMatch = i.material.includes(order.material) || order.material.includes(i.material);
        return colorMatch && materialMatch;
    });

    if (relevantInv.length === 0) {
        alert("?깅줉???대떦 ?꾨씪硫섑듃 ?ш퀬 ?뺣낫媛 ?놁뒿?덈떎.");
        return;
    }

    alert(`珥?${relevantInv.length}媛쒖쓽 愿???꾨씪硫섑듃媛 寃?됰릺?덉뒿?덈떎.\n吏湲덈????섎굹??臾닿쾶瑜??낅젰諛쏆뒿?덈떎. 蹂寃쎌쓣 ?먯튂 ?딅뒗 ??ぉ? 鍮덉뭏?쇰줈 ?먭굅??[痍⑥냼]瑜??꾨Ⅴ?몄슂.`);

    for (const inv of relevantInv) {
        const currentEst = (inv.remaining_weight_g || 0);
        const weight = prompt(`[${inv.material} / ${inv.color}] (ID: ${inv.id})\n?꾩옱 湲곕줉???붾웾: ${currentEst}g\n\n?덈줈??痢≪젙 臾닿쾶(g)瑜??낅젰?섏꽭??`, "");
        
        // 鍮덉뭏???꾨땲怨??レ옄??寃쎌슦?먮쭔 ?낅뜲?댄듃
        if (weight !== null && weight !== "" && !isNaN(weight)) {
            await postData(`/api/v1/inventory/${inv.id}`, { remaining_weight_g: parseFloat(weight) }, 'PATCH');
        }
    }
    
    alert("紐⑤뱺 ?ㅼ륫 ?붾웾 湲곕줉 ?덉감媛 ?꾨즺?섏뿀?듬땲??");
    fetchData();
}

async function postData(url, data, method='POST') { return fetch(url, { method: method, headers: API_MUTATION_HEADERS, body: JSON.stringify(data) }); }
async function deleteOrder(id) {
    if (confirm('二쇰Ц????젣?섏떆寃좎뒿?덇퉴?')) {
        const deleteSchedules = confirm('??二쇰Ц怨??곌껐??紐⑤뱺 ?앹궛 怨꾪쉷(?쇱젙)??媛숈씠 ??젣?섏떆寃좎뒿?덇퉴?\n\n[?뺤씤] : 怨꾪쉷????젣\n[痍⑥냼] : 二쇰Ц留???젣 (怨꾪쉷 ?좎?)');
        await fetch(`/api/v1/orders/${id}?delete_schedules=${deleteSchedules}`, { method: 'DELETE', headers: { 'X-Requested-With': 'XMLHttpRequest' } });
        fetchData();
    }
}
async function deleteSchedule(id) { if (confirm('怨꾪쉷 ??젣?')) { await fetch(`/api/v1/schedules/${id}`, { method: 'DELETE', headers: { 'X-Requested-With': 'XMLHttpRequest' } }); fetchData(); } }
async function deletePrinter(id) { if(confirm('?꾨┛????젣?')) { await fetch(`/api/v1/printers/${id}`, { method: 'DELETE', headers: { 'X-Requested-With': 'XMLHttpRequest' } }); fetchData(); } }
async function deleteInventory(id) { if(confirm('?ш퀬 ??젣?')) { await fetch(`/api/v1/inventory/${id}`, { method: 'DELETE', headers: { 'X-Requested-With': 'XMLHttpRequest' } }); fetchData(); } }

// --- Global Events for Order Form ---
function initOrderFormEvents() {
    const productInput = document.getElementById('order_product');
    const materialSelect = document.getElementById('order_material');
    const colorSelect = document.getElementById('order_color');
    const weightInput = document.getElementById('order_unit_weight');

    // 1. ?쒗뭹紐??낅젰 ???댁쟾 臾닿쾶 異붿쿇
    productInput.addEventListener('blur', () => {
        const val = productInput.value.trim();
        if (!val) return;

        // 湲곗〈 二쇰Ц?먯꽌 ?숈씪 ?쒗뭹紐?李얘린
        const pastOrder = (currentData.orders || []).find(o => o.product_name === val);
        if (pastOrder && pastOrder.unit_weight_g) {
            if (confirm(`'${val}' ?쒗뭹??湲곗〈 湲곕줉??媛쒕떦 臾닿쾶??${pastOrder.unit_weight_g}g ?낅땲?? ??臾닿쾶瑜??ъ슜?섏떆寃좎뒿?덇퉴?`)) {
                weightInput.value = pastOrder.unit_weight_g;
            }
        }
    });

    // 2. ?ъ쭏 ?좏깮 ???됱긽 ?먮룞 ?좏깮 諛??덈궡
    materialSelect.addEventListener('change', () => {
        const selectedId = materialSelect.value;
        const inv = (currentData.inventory || []).find(i => i.id === selectedId);
        if (inv) {
            // ?됱긽 ?좏깮 ?숆린??
            colorSelect.value = inv.color;
            // ?덈궡 ?앹뾽 (?붾웾 ?뺣낫)
            alert(`?좏깮?섏떊 [${inv.material} / ${inv.color}]???꾩옱 ?쒖뒪???붾웾? ${inv.remaining_weight_g}g ?낅땲??`);
        }
    });
}

function updateSelects(orders, printers, inventory) {
    const os = document.getElementById('schedule_order_id');
    const ps = document.getElementById('printer_id');
    
    // 二쇰Ц ?깅줉 ?쇱슜 ??됲듃諛뺤뒪
    const om = document.getElementById('order_material');
    const oc = document.getElementById('order_color');
    const em = document.getElementById('edit_order_material');
    const ec = document.getElementById('edit_order_color');

    const curP = ps.value;
    
    // ?묒뾽 ?좊떦??二쇰Ц ?좏깮
    if (orders) os.innerHTML = '<option value="">-- 二쇰Ц ?좏깮 --</option>' + orders.filter(o => o.status !== 'completed').map(o => `<option value="${o.id}">${o.product_name}</option>`).join('');
    
    // ?꾨┛???좏깮
    if (printers) ps.innerHTML = '<option value="">-- ?꾨┛???좏깮 --</option>' + printers.map(p => `<option value="${p.id}">${p.model} (${p.asset_id})</option>`).join('');
    if (curP) ps.value = curP;

    // ?ш퀬 湲곕컲 ?ъ쭏/?됱긽 ?좏깮 (二쇰Ц ?깅줉??
    if (inventory) {
        const materialOptions = '<option value="">-- ?ъ쭏 ?좏깮 --</option>' + inventory.map(i => `<option value="${i.id}">${i.material} (${i.color})</option>`).join('');
        const colorOptions = '<option value="">-- ?됱긽 ?좏깮 --</option>' + [...new Set(inventory.map(i => i.color))].map(c => `<option value="${c}">${c}</option>`).join('');
        
        if (om) om.innerHTML = materialOptions;
        if (em) em.innerHTML = materialOptions;
        if (oc) oc.innerHTML = colorOptions;
        if (ec) ec.innerHTML = colorOptions;
    }
}

// --- Helper Logic ---
function getColorStyle(colorName) {
    const colorMap = {
        '諛깆깋': '#ffffff', '?붿씠??: '#ffffff', 'white': '#ffffff',
        '寃??: '#000000', '釉붾옓': '#000000', 'black': '#000000',
        '?뚯깋': '#808080', '洹몃젅??: '#808080', 'grey': '#808080', 'gray': '#808080',
        '鍮④컯': '#ff0000', '?덈뱶': '#ff0000', 'red': '#ff0000',
        '?뚮옉': '#0000ff', '釉붾（': '#0000ff', 'blue': '#0000ff',
        '?뱀깋': '#28a745', '洹몃┛': '#28a745', 'green': '#28a745',
        '?몃옉': '#ffc107', '?먮줈??: '#ffc107', 'yellow': '#ffc107',
        '?ㅻ젋吏': '#fd7e14', 'orange': '#fd7e14',
        '蹂대씪': '#6f42c1', 'purple': '#6f42c1',
        '?щ챸': '#e9ecef', 'clear': '#e9ecef'
    };

    const bgColor = colorMap[colorName] || '#dee2e6'; // 留ㅽ븨 ?덈릺硫??고븳 ?뚯깋

    // 諛앷린 怨꾩궛 (Luma: 0.299R + 0.587G + 0.114B)
    // HEX瑜?RGB濡?蹂?섑븯??怨꾩궛
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
    
    // 1. ?ъ쭏/?됱긽蹂??곗씠??吏묎퀎 (紐⑤뱺 諛곗튂 ?듯빀)
    const stats = {}; 

    // ?⑤?由????앹꽦 ?⑥닔 (?ъ쭏紐낃낵 ?됱긽留뚯쑝濡?洹몃９??
    const getFamilyKey = (item) => {
        const m = item.material.toLowerCase();
        const c = item.color.toLowerCase();
        
        // 二쇰Ц?먯꽌 ?ъ슜?섎뒗 ????ъ쭏紐?李얘린
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

    // ?ш퀬 湲곕컲?쇰줈 ?꾩옱 異붿젙 ?붾웾 怨꾩궛 諛?'?꾩껜 ?⑹궛'
    inv.forEach(i => {
        const s = getStats(i);
        if (i.batch) s.batches.add(i.batch);
        else s.batches.add(i.material); // 諛곗튂媛 ?놁쑝硫??ъ쭏紐낆씠?쇰룄 ?ｌ뼱???앸퀎
        
        const lastUpdate = i.updated_at ? new Date(i.updated_at) : new Date(0);
        const consumedAfterUpdate = (currentData.schedules || [])
            .filter(sch => {
                const order = (currentData.orders || []).find(o => o.id === sch.order_id);
                if (!order || order.color.toLowerCase() !== i.color.toLowerCase()) return false;
                const m1 = i.material.toLowerCase();
                const m2 = order.material.toLowerCase();
                if (!(m1.includes(m2) || m2.includes(m1))) return false;
                const actionTime = sch.created_at ? new Date(sch.created_at) : new Date(0);
                return actionTime > lastUpdate;
            })
            .reduce((sum, sch) => sum + (parseFloat(sch.actual_quantity) || 0), 0);
        
        const orderForWeight = currentData.orders.find(o => (i.material.toLowerCase().includes(o.material.toLowerCase()) || o.material.toLowerCase().includes(i.material.toLowerCase())) && o.color.toLowerCase() === i.color.toLowerCase()) || {unit_weight_g:0};
        const itemEstimatedStock = parseFloat(i.remaining_weight_g || 0) - (consumedAfterUpdate * (orderForWeight.unit_weight_g || 0));
        
        s.estimatedCurrentStock += itemEstimatedStock;
    });

    // ?ν썑 ?꾩슂??怨꾩궛 (?ъ쭏 洹몃９蹂꾨줈 ????踰덈쭔 怨꾩궛)
    orders.filter(o => o.status !== 'completed').forEach(o => {
        const key = `${o.material.toLowerCase()}_${o.color.toLowerCase()}`;
        if (stats[key]) {
            const actualSum = (currentData.schedules||[]).filter(sch => sch.order_id === o.id).reduce((sum, sch) => sum + (parseFloat(sch.actual_quantity)||0), 0);
            const remainingQty = Math.max(0, o.target_quantity - o.initial_stock - actualSum);
            stats[key].future += remainingQty * (o.unit_weight_g || 0);
        }
    });

    // 2. ?뚮뜑留?
    const sortedInv = inv.slice().sort((a, b) => {
        const matCompare = a.material.localeCompare(b.material, undefined, {numeric: true, sensitivity: 'base'});
        if (matCompare !== 0) return matCompare;
        return a.color.localeCompare(b.color, undefined, {numeric: true, sensitivity: 'base'});
    });

    sortedInv.forEach(i => {
        const s = getStats(i);
        
        // 媛쒕퀎 ?꾩씠???쒖떆???붾웾 怨꾩궛
        const lastUpdate = i.updated_at ? new Date(i.updated_at) : new Date(0);
        const orderForWeight = currentData.orders.find(o => (i.material.toLowerCase().includes(o.material.toLowerCase()) || o.material.toLowerCase().includes(i.material.toLowerCase())) && o.color.toLowerCase() === i.color.toLowerCase()) || {unit_weight_g:0};
        const consumedAfter = (currentData.schedules || [])
            .filter(sch => {
                const order = (currentData.orders || []).find(o => o.id === sch.order_id);
                const isMatch = order && (i.material.toLowerCase().includes(order.material.toLowerCase()) || order.material.toLowerCase().includes(i.material.toLowerCase())) && order.color.toLowerCase() === i.color.toLowerCase();
                return isMatch && (sch.created_at ? new Date(sch.created_at) : new Date(0)) > lastUpdate;
            })
            .reduce((sum, sch) => sum + (parseFloat(sch.actual_quantity) || 0), 0);
        
        const itemEstimatedStock = i.remaining_weight_g - (consumedAfter * orderForWeight.unit_weight_g);
        
        const isShort = s.estimatedCurrentStock < s.future;
        const tr = document.createElement('tr');
        if (isShort) tr.className = 'conflict-row';
        
        let statusHtml = '';
        if (s.future > 0) {
            const batchStr = Array.from(s.batches).sort().join('+');
            const totalStock = s.estimatedCurrentStock.toFixed(1);
            const needed = s.future.toFixed(1);
            
            if (isShort) {
                statusHtml = `<br><small style="color:red; font-weight:bold;">[遺議? ?듯빀(${batchStr}): ${totalStock}g / ?꾩슂: ${needed}g</small>`;
            } else {
                const surplus = (s.estimatedCurrentStock - s.future).toFixed(1);
                statusHtml = `<br><small style="color:green; font-weight:bold;">[異⑸텇] ?듯빀(${batchStr}): ${totalStock}g / ?ъ쑀: ${surplus}g</small>`;
            }
        }

        const batchBadge = i.batch ? `<span style="background:#6f42c1; color:white; padding:2px 6px; border-radius:4px; font-size:0.8em; margin-right:5px;">${i.batch}</span>` : '';

        tr.innerHTML = `
            <td>${batchBadge}${i.material}</td>
            <td><span style="${getColorStyle(i.color)}">${i.color}</span></td>
            <td>
                <span title="?깅줉 臾닿쾶: ${i.remaining_weight_g}g">${itemEstimatedStock.toFixed(1)}g</span>
                ${statusHtml}
            </td>
            <td><small>${i.updated_at || '-'}</small></td>
            <td>
                <button onclick="openInventoryEdit('${i.id}')" class="btn-sm">?섏젙</button>
                <button onclick="deleteInventory('${i.id}')" class="btn-sm btn-danger">??젣</button>
            </td>`;
        tb.appendChild(tr);
    });
}

function renderPrinters(list) {
    const tb = document.getElementById('printer-body'); tb.innerHTML = '';
    list.forEach(p => {
        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${p.model}</td><td>${p.asset_id}</td><td>${p.purchase_date}</td>
            <td><button onclick="openPrinterEdit('${p.id}')" class="btn-sm">?섏젙</button><button onclick="deletePrinter('${p.id}')" class="btn-sm btn-danger">??젣</button></td>`;
        tb.appendChild(tr);
    });
}

function renderDashboard(data) {
    const totalTarget = (data.orders||[]).reduce((sum, o) => sum + parseFloat(o.target_quantity), 0);
    const totalSecured = (data.orders||[]).reduce((sum, o) => sum + parseFloat(o.initial_stock) + parseFloat(o.completed_quantity), 0);
    const percent = totalTarget ? Math.floor((totalSecured/totalTarget)*100) : 0;
    document.querySelector('#stat-progress .data-value').textContent = `${percent}%`;
    document.querySelector('#stat-printers .data-value').textContent = (data.printers||[]).length;
}

function renderTimeline(data) {
    const header = document.getElementById('gantt-header');
    const body = document.getElementById('gantt-body');
    if (!header || !body) return;
    
    header.innerHTML = '<th>?꾨┛??\\ ?쒓컙</th>';
    body.innerHTML = '';

    // 理쒓렐 7?쇨컙???좎쭨 ?ㅻ뜑 ?앹꽦
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
                tr.innerHTML += `<td style="background:${color}; font-size:0.8em;">${order ? order.product_name : '?묒뾽'}</td>`;
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

