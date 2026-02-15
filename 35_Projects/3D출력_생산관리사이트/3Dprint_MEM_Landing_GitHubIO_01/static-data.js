window.DEMO_DATA = {
  orders: [
    { id: "o1", product_name: "LED 커버 바디", target_quantity: 200, initial_stock: 40, completed_quantity: 96, status: "in_progress", material: "PLA", color: "Black", unit_weight_g: 9.2, deadline: "2026-03-03" },
    { id: "o2", product_name: "센서 브래킷", target_quantity: 80, initial_stock: 10, completed_quantity: 70, status: "completed", material: "PETG", color: "White", unit_weight_g: 14.1, deadline: "2026-02-24" },
    { id: "o3", product_name: "케이블 가이드", target_quantity: 150, initial_stock: 20, completed_quantity: 45, status: "in_progress", material: "PLA", color: "Gray", unit_weight_g: 6.8, deadline: "2026-03-10" }
  ],
  schedules: [
    { id: "s1", order_id: "o1", printer: "P2SC-002", planned_quantity: 30, actual_quantity: 24, start_date: "2026-02-12", status: "in_progress" },
    { id: "s2", order_id: "o3", printer: "P2SC-005", planned_quantity: 25, actual_quantity: 0, start_date: "2026-02-13", status: "pending" },
    { id: "s3", order_id: "o2", printer: "P2SC-003", planned_quantity: 20, actual_quantity: 20, start_date: "2026-02-11", status: "completed" }
  ],
  inventory: [
    { material: "PLA", color: "Black", remaining_weight_g: 720, risk: "ok" },
    { material: "PLA", color: "Gray", remaining_weight_g: 180, risk: "warn" },
    { material: "PETG", color: "White", remaining_weight_g: 95, risk: "danger" }
  ],
  printers: [
    { model: "P2SC", asset_id: "002" },
    { model: "P2SC", asset_id: "003" },
    { model: "P2SC", asset_id: "005" }
  ]
};
