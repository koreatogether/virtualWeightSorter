(function () {
  const data = window.DEMO_DATA;

  function fmt(n) {
    return Number(n || 0).toLocaleString();
  }

  function renderStats() {
    const totalOrders = data.orders.length;
    const activeOrders = data.orders.filter(o => o.status !== "completed").length;
    const totalPrinters = data.printers.length;
    const avgProgress = Math.round(
      data.orders.reduce((sum, o) => {
        const secured = (o.initial_stock || 0) + (o.completed_quantity || 0);
        return sum + Math.min(100, (secured / (o.target_quantity || 1)) * 100);
      }, 0) / totalOrders
    );

    document.getElementById("stats").innerHTML = `
      <article class="stat"><div class="k">총 주문</div><div class="v">${fmt(totalOrders)}</div></article>
      <article class="stat"><div class="k">진행 주문</div><div class="v">${fmt(activeOrders)}</div></article>
      <article class="stat"><div class="k">가동 프린터</div><div class="v">${fmt(totalPrinters)}</div></article>
      <article class="stat"><div class="k">평균 진행률</div><div class="v">${fmt(avgProgress)}%</div></article>
    `;
  }

  function renderOrders() {
    const html = data.orders.map(o => {
      const secured = (o.initial_stock || 0) + (o.completed_quantity || 0);
      const progress = Math.min(100, Math.round((secured / (o.target_quantity || 1)) * 100));
      return `
        <article class="order-card">
          <div class="order-title">${o.product_name}</div>
          <div class="progress-bg"><div class="progress-fill" style="width:${progress}%"></div></div>
          <div class="meta">목표 ${fmt(o.target_quantity)} / 확보 ${fmt(secured)} / 진행률 ${progress}%</div>
          <div class="meta">${o.material} ${o.color} · ${o.unit_weight_g}g/ea · 마감 ${o.deadline}</div>
        </article>
      `;
    }).join("");

    document.getElementById("order-list").innerHTML = html;
  }

  function badge(status) {
    if (status === "completed") return '<span class="badge ok">완료</span>';
    if (status === "in_progress") return '<span class="badge warn">진행중</span>';
    return '<span class="badge">대기</span>';
  }

  function riskBadge(risk) {
    if (risk === "danger") return '<span class="badge danger">부족</span>';
    if (risk === "warn") return '<span class="badge warn">주의</span>';
    return '<span class="badge ok">정상</span>';
  }

  function renderSchedules() {
    const orderMap = Object.fromEntries(data.orders.map(o => [o.id, o.product_name]));
    const rows = data.schedules.map(s => `
      <tr>
        <td>${s.start_date}</td>
        <td>${orderMap[s.order_id] || "-"}</td>
        <td>${s.printer}</td>
        <td>${fmt(s.planned_quantity)} / ${fmt(s.actual_quantity)}</td>
        <td>${badge(s.status)}</td>
      </tr>
    `).join("");
    document.getElementById("schedule-body").innerHTML = rows;
  }

  function renderInventory() {
    const rows = data.inventory.map(i => `
      <tr>
        <td>${i.material}</td>
        <td>${i.color}</td>
        <td>${fmt(i.remaining_weight_g)}</td>
        <td>${riskBadge(i.risk)}</td>
      </tr>
    `).join("");
    document.getElementById("inventory-body").innerHTML = rows;
  }

  renderStats();
  renderOrders();
  renderSchedules();
  renderInventory();
})();
