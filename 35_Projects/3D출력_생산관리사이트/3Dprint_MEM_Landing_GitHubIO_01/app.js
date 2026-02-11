(function () {
  const data = window.DEMO_DATA ?? { orders: [], printers: [], schedules: [], inventory: [] };

  /**
   * 숫자 포맷팅 (한국 로케일)
   */
  function fmt(n) {
    if (n === undefined || n === null) return "0";
    return Number(n).toLocaleString('ko-KR');
  }

  /**
   * 통계 요약 렌더링
   */
  function renderStats() {
    const container = document.getElementById("stats");
    if (!container) return;

    const totalOrders = data.orders?.length || 0;
    const activeOrders = data.orders?.filter(o => o.status !== "completed").length || 0;
    const totalPrinters = data.printers?.length || 0;

    // 0으로 나누기 방지
    let avgProgress = 0;
    if (totalOrders > 0) {
      avgProgress = Math.round(
        data.orders.reduce((sum, o) => {
          const target = o.target_quantity || 1;
          const secured = (o.initial_stock || 0) + (o.completed_quantity || 0);
          return sum + Math.min(100, (secured / target) * 100);
        }, 0) / totalOrders
      );
    }

    container.innerHTML = `
      <h2 id="stats-title" class="visually-hidden">통계 요약</h2>
      <article class="stat"><div class="k">총 주문</div><div class="v">${fmt(totalOrders)}</div></article>
      <article class="stat"><div class="k">진행 주문</div><div class="v">${fmt(activeOrders)}</div></article>
      <article class="stat"><div class="k">가동 프린터</div><div class="v">${fmt(totalPrinters)}</div></article>
      <article class="stat"><div class="k">평균 진행률</div><div class="v">${fmt(avgProgress)}%</div></article>
    `;
  }

  /**
   * 주문 현황 (카드 그리드) 렌더링
   */
  function renderOrders() {
    const container = document.getElementById("order-list");
    if (!container || !data.orders) return;

    const fragment = document.createDocumentFragment();

    data.orders.forEach(o => {
      const target = o.target_quantity || 1;
      const secured = (o.initial_stock || 0) + (o.completed_quantity || 0);
      const progress = Math.min(100, Math.round((secured / target) * 100));

      const article = document.createElement("article");
      article.className = "order-card";
      article.innerHTML = `
        <div class="order-title">${o.product_name}</div>
        <div class="progress-bg"><div class="progress-fill" style="width:${progress}%"></div></div>
        <div class="meta">목표 ${fmt(o.target_quantity)} / 확보 ${fmt(secured)} / 진행률 ${progress}%</div>
        <div class="meta">${o.material} ${o.color} · ${o.unit_weight_g}g/ea · 마감 ${o.deadline}</div>
      `;
      fragment.appendChild(article);
    });

    container.innerHTML = "";
    container.appendChild(fragment);
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

  /**
   * 생산 일정 테이블 렌더링
   */
  function renderSchedules() {
    const container = document.getElementById("schedule-body");
    if (!container || !data.schedules) return;

    const orderMap = Object.fromEntries(data.orders.map(o => [o.id, o.product_name]));

    const rowsHtml = data.schedules.map(s => `
      <tr>
        <td>${s.start_date || "-"}</td>
        <td>${orderMap[s.order_id] || "-"}</td>
        <td>${s.printer || "-"}</td>
        <td>${fmt(s.planned_quantity)} / ${fmt(s.actual_quantity)}</td>
        <td>${badge(s.status)}</td>
      </tr>
    `).join("");

    container.innerHTML = rowsHtml;
  }

  /**
   * 재고 현황 테이블 렌더링
   */
  function renderInventory() {
    const container = document.getElementById("inventory-body");
    if (!container || !data.inventory) return;

    const rowsHtml = data.inventory.map(i => `
      <tr>
        <td>${i.material}</td>
        <td>${i.color}</td>
        <td>${fmt(i.remaining_weight_g)}</td>
        <td>${riskBadge(i.risk)}</td>
      </tr>
    `).join("");

    container.innerHTML = rowsHtml;
  }

  // 초기 실행
  try {
    renderStats();
    renderOrders();
    renderSchedules();
    renderInventory();
  } catch (e) {
    console.error("Data rendering failed:", e);
  }
})();
