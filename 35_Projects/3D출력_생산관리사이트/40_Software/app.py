from flask import Flask, render_template, jsonify, request
import json
import os
import uuid
from datetime import datetime, timedelta

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_FILE = os.path.join(BASE_DIR, 'data', 'db.json')
ACTION_LOG_FILE = os.path.join(BASE_DIR, 'data', 'user_actions.log')

def log_action(action_type, target, details):
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    ip = request.remote_addr
    log_entry = f"[{timestamp}] [IP: {ip}] [ACTION: {action_type}] [TARGET: {target}] - {details}\n"
    with open(ACTION_LOG_FILE, 'a', encoding='utf-8') as f:
        f.write(log_entry)

def read_db():
    if not os.path.exists(DB_FILE):
        os.makedirs(os.path.dirname(DB_FILE), exist_ok=True)
        default_data = {"orders": [], "schedules": [], "inventory": [], "printers": []}
        write_db(default_data)
        return default_data
    with open(DB_FILE, 'r', encoding='utf-8') as f:
        try:
            data = json.load(f)
            if 'orders' not in data: data['orders'] = []
            return data
        except json.JSONDecodeError:
            return {"orders": [], "schedules": [], "inventory": [], "printers": []}

def write_db(data):
    with open(DB_FILE, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)

def check_conflict(schedules, printer_id, date_str):
    target_date = date_str.split('T')[0]
    for s in schedules:
        if s.get('printer_id') == printer_id:
            s_date = s.get('start_time', '').split('T')[0]
            if s_date == target_date:
                return True
    return False

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/v1/data', methods=['GET'])
def get_data():
    return jsonify(read_db())

# --- 주문 (Orders) API ---
@app.route('/api/v1/orders', methods=['POST'])
def add_order():
    data = read_db()
    new_order = request.json
    new_order['id'] = str(uuid.uuid4())[:8]
    new_order['initial_stock'] = int(new_order.get('initial_stock', 0))
    new_order['completed_quantity'] = 0
    new_order['status'] = 'in_progress'
    
    # 필라멘트 정보 (새로 추가됨)
    new_order['material'] = new_order.get('material', 'PLA')
    new_order['color'] = new_order.get('color', 'Black')
    new_order['unit_weight_g'] = int(new_order.get('unit_weight_g', 0))
    
    new_order['created_at'] = datetime.now().isoformat()
    data['orders'].append(new_order)
    write_db(data)
    log_action("CREATE_ORDER", new_order['id'], f"Product: {new_order['product_name']}, Target: {new_order['target_quantity']}")
    return jsonify({"status": "success", "id": new_order['id']}), 201

@app.route('/api/v1/orders/<id>', methods=['PATCH'])
def update_order(id):
    data = read_db()
    req_data = request.json
    for order in data['orders']:
        if order['id'] == id:
            if 'product_name' in req_data: order['product_name'] = req_data['product_name']
            if 'target_quantity' in req_data: order['target_quantity'] = int(req_data['target_quantity'])
            if 'initial_stock' in req_data: order['initial_stock'] = int(req_data['initial_stock'])
            if 'deadline' in req_data: order['deadline'] = req_data['deadline']
            
            # 필라멘트 정보 수정
            if 'material' in req_data: order['material'] = req_data['material']
            if 'color' in req_data: order['color'] = req_data['color']
            if 'unit_weight_g' in req_data: order['unit_weight_g'] = int(req_data['unit_weight_g'])
            
            actual_sum = sum(s.get('actual_quantity', 0) for s in data['schedules'] if s.get('order_id') == id)
            if (actual_sum + order.get('initial_stock', 0)) >= int(order['target_quantity']):
                order['status'] = 'completed'
            else:
                order['status'] = 'in_progress'
            
            write_db(data)
            log_action("UPDATE_ORDER", id, f"Updated fields: {list(req_data.keys())}")
            return jsonify({"status": "success"})
    return jsonify({"status": "error", "message": "Order not found"}), 404

@app.route('/api/v1/orders/<id>', methods=['DELETE'])
def delete_order(id):
    data = read_db()
    data['orders'] = [o for o in data['orders'] if o['id'] != id]
    write_db(data)
    log_action("DELETE_ORDER", id, "Order removed")
    return jsonify({"status": "success"})

# --- 자동 스케줄링 API ---
@app.route('/api/v1/schedules/auto', methods=['POST'])
def auto_schedule():
    data = read_db()
    req = request.json
    order_id = req.get('order_id')
    printer_id = req.get('printer_id')
    daily_qty = int(req.get('daily_qty'))
    start_date_str = req.get('start_date')
    
    target_order = next((o for o in data['orders'] if o['id'] == order_id), None)
    if not target_order: return jsonify({"status": "error", "message": "Order not found"}), 404
        
    remaining_qty = int(target_order['target_quantity']) - int(target_order.get('initial_stock', 0)) - int(target_order.get('completed_quantity', 0))
    if remaining_qty <= 0: return jsonify({"status": "error", "message": "Order already completed"}), 400

    created_schedules = []
    conflicts = []
    try:
        current_date = datetime.strptime(start_date_str.split('T')[0], '%Y-%m-%d')
    except ValueError:
        return jsonify({"status": "error", "message": "Invalid date format"}), 400

    while remaining_qty > 0:
        plan_qty = min(remaining_qty, daily_qty)
        current_date_str = current_date.strftime('%Y-%m-%dT09:00')
        is_conflict = check_conflict(data['schedules'], printer_id, current_date_str) or \
                      check_conflict(created_schedules, printer_id, current_date_str)
        
        new_task = {
            "id": str(uuid.uuid4())[:8],
            "order_id": order_id,
            "printer_id": printer_id,
            "planned_quantity": plan_qty,
            "actual_quantity": 0,
            "status": "pending",
            "start_time": current_date_str,
            "created_at": datetime.now().isoformat(),
            "conflict": is_conflict
        }
        if is_conflict: conflicts.append(current_date_str.split('T')[0])
        data['schedules'].append(new_task)
        created_schedules.append(new_task)
        remaining_qty -= plan_qty
        current_date += timedelta(days=1)
        
    write_db(data)
    log_action("AUTO_SCHEDULE", order_id, f"Created {len(created_schedules)} days for printer {printer_id}")
    msg = f"{len(created_schedules)}일치 생성 완료."
    if conflicts: msg += f" (경고: {len(conflicts)}건 중복)"
    return jsonify({"status": "success", "count": len(created_schedules), "message": msg, "conflicts": conflicts}), 201

# --- 일정 (Production Runs) API ---
@app.route('/api/v1/schedules', methods=['POST'])
def add_schedule():
    data = read_db()
    new_task = request.json
    new_task['id'] = str(uuid.uuid4())[:8]
    new_task['status'] = 'pending'
    new_task['actual_quantity'] = 0
    new_task['created_at'] = datetime.now().isoformat()
    if not new_task.get('start_time'):
        new_task['start_time'] = datetime.now().strftime('%Y-%m-%dT%H:%M')
    if check_conflict(data['schedules'], new_task.get('printer_id'), new_task['start_time']):
        new_task['conflict'] = True
    else:
        new_task['conflict'] = False
    data['schedules'].append(new_task)
    write_db(data)
    log_action("ADD_SCHEDULE", new_task['id'], f"Order: {new_task.get('order_id')}, Printer: {new_task.get('printer_id')}")
    return jsonify({"status": "success", "id": new_task['id'], "conflict": new_task['conflict']}), 201

@app.route('/api/v1/schedules/<id>', methods=['PATCH'])
def update_schedule(id):
    data = read_db()
    req_data = request.json
    target_schedule = None
    
    # 1. 대상 스케줄 찾기
    for item in data['schedules']:
        if item['id'] == id:
            target_schedule = item
            # 만약 actual_quantity가 요청에 포함되어 있다면, 기존 값에 더해줌 (누적 처리)
            if 'actual_quantity' in req_data:
                added_qty = int(req_data['actual_quantity'])
                old_actual = int(item.get('actual_quantity', 0))
                item['actual_quantity'] = old_actual + added_qty
                
                # 계획 수량보다 초과해서 생산했는지 확인
                planned = int(item.get('planned_quantity', 0))
                excess = item['actual_quantity'] - planned
                
                # 만약 초과분이 있다면, 동일 주문의 다음 일정들에서 차감
                if excess > 0:
                    order_id = item.get('order_id')
                    # 동일 주문의 완료되지 않은(pending) 미래 일정들을 시간순으로 가져옴
                    future_schedules = sorted(
                        [s for s in data['schedules'] if s.get('order_id') == order_id and s.get('status') == 'pending' and s['id'] != id],
                        key=lambda x: x.get('start_time', '')
                    )
                    
                    for fs in future_schedules:
                        if excess <= 0: break
                        fs_planned = int(fs.get('planned_quantity', 0))
                        can_reduce = min(fs_planned, excess)
                        fs['planned_quantity'] = fs_planned - can_reduce
                        excess -= can_reduce
                        
                        # 만약 계획 수량이 0이 되면 해당 일정은 더 이상 필요 없으므로 자동 완료 처리하거나 고민 필요
                        # 여기서는 일단 0으로 두고 유지 (사용자가 보게 함)
            
            # 나머지 필드 업데이트
            for key, value in req_data.items():
                if key != 'actual_quantity': # 실적은 위에서 누적처리함
                    item[key] = value
            break

    if target_schedule:
        order_id = target_schedule.get('order_id')
        if order_id:
            total_completed = sum(s.get('actual_quantity', 0) for s in data['schedules'] if s.get('order_id') == order_id)
            for order in data['orders']:
                if order['id'] == order_id:
                    order['completed_quantity'] = total_completed
                    if (order['completed_quantity'] + order.get('initial_stock', 0)) >= int(order['target_quantity']):
                        order['status'] = 'completed'
                    else:
                        order['status'] = 'in_progress'
                    break
        write_db(data)
        log_action("UPDATE_SCHEDULE_ACTUAL", id, f"New actual total: {target_schedule.get('actual_quantity')}")
        return jsonify({"status": "success", "new_actual": target_schedule.get('actual_quantity')})
    return jsonify({"status": "error", "message": "Not found"}), 404

@app.route('/api/v1/schedules/<id>', methods=['DELETE'])
def delete_schedule(id):
    data = read_db()
    order_id = None
    for s in data['schedules']:
        if s['id'] == id:
            order_id = s.get('order_id')
            break
    if order_id:
        data['schedules'] = [s for s in data['schedules'] if s['id'] != id]
        total_completed = sum(s.get('actual_quantity', 0) for s in data['schedules'] if s.get('order_id') == order_id)
        for order in data['orders']:
            if order['id'] == order_id:
                order['completed_quantity'] = total_completed
                if (order['completed_quantity'] + order.get('initial_stock', 0)) >= int(order['target_quantity']):
                    order['status'] = 'completed'
                else:
                    order['status'] = 'in_progress'
                break
        write_db(data)
        log_action("DELETE_SCHEDULE", id, "Schedule removed")
        return jsonify({"status": "success"})
    return jsonify({"status": "error", "message": "Not found"}), 404

# --- 재고 및 프린터 API ---
@app.route('/api/v1/inventory', methods=['POST'])
def add_inventory():
    data = read_db()
    new_inv = request.json
    new_inv['id'] = str(uuid.uuid4())[:8]
    new_inv['updated_at'] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    data['inventory'].append(new_inv)
    write_db(data)
    log_action("ADD_INVENTORY", new_inv['id'], f"Material: {new_inv['material']}, Weight: {new_inv['remaining_weight_g']}g")
    return jsonify({"status": "success", "id": new_inv['id']}), 201

@app.route('/api/v1/inventory/<id>', methods=['PATCH'])
def update_inventory(id):
    data = read_db()
    req_data = request.json
    for item in data['inventory']:
        if item['id'] == id:
            if 'material' in req_data: item['material'] = req_data['material']
            if 'color' in req_data: item['color'] = req_data['color']
            if 'remaining_weight_g' in req_data: item['remaining_weight_g'] = int(req_data['remaining_weight_g'])
            item['updated_at'] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            write_db(data)
            log_action("UPDATE_INVENTORY", id, f"Updated fields: {list(req_data.keys())}")
            return jsonify({"status": "success"})
    return jsonify({"status": "error", "message": "Inventory not found"}), 404

@app.route('/api/v1/inventory/<id>', methods=['DELETE'])
def delete_inventory(id):
    data = read_db()
    data['inventory'] = [i for i in data['inventory'] if i['id'] != id]
    write_db(data)
    log_action("DELETE_INVENTORY", id, "Inventory removed")
    return jsonify({"status": "success"})

@app.route('/api/v1/printers', methods=['POST'])
def add_printer():
    data = read_db()
    new_printer = request.json
    new_printer['id'] = str(uuid.uuid4())[:8]
    if not new_printer.get('asset_id'):
        new_printer['asset_id'] = f"{new_printer.get('model', 'PRN')}-{str(uuid.uuid4())[:4]}"
    new_printer['status'] = 'idle'
    if not new_printer.get('purchase_date'):
        new_printer['purchase_date'] = datetime.now().strftime('%Y-%m-%d')
    data['printers'].append(new_printer)
    write_db(data)
    log_action("ADD_PRINTER", new_printer['id'], f"Model: {new_printer['model']}, AssetID: {new_printer['asset_id']}")
    return jsonify({"status": "success", "id": new_printer['id']}), 201

@app.route('/api/v1/printers/<id>', methods=['PATCH'])
def update_printer(id):
    data = read_db()
    req_data = request.json
    for item in data['printers']:
        if item['id'] == id:
            if 'model' in req_data: item['model'] = req_data['model']
            if 'asset_id' in req_data: item['asset_id'] = req_data['asset_id']
            if 'purchase_date' in req_data: item['purchase_date'] = req_data['purchase_date']
            write_db(data)
            log_action("UPDATE_PRINTER", id, f"Updated fields: {list(req_data.keys())}")
            return jsonify({"status": "success"})
    return jsonify({"status": "error", "message": "Printer not found"}), 404

@app.route('/api/v1/printers/<id>', methods=['DELETE'])
def delete_printer(id):
    data = read_db()
    data['printers'] = [p for p in data['printers'] if p['id'] != id]
    write_db(data)
    log_action("DELETE_PRINTER", id, "Printer removed")
    return jsonify({"status": "success"})

@app.route('/api/v1/printers/<id>/busy_dates', methods=['GET'])
def get_printer_busy_dates(id):
    data = read_db()
    busy_dates = set()
    for s in data['schedules']:
        if s.get('printer_id') == id and s.get('status') != 'completed': # 완료된건 제외하고 예정/진행중인 날짜만
            date_str = s.get('start_time', '').split('T')[0]
            if date_str:
                busy_dates.add(date_str)
    return jsonify(list(busy_dates))

if __name__ == '__main__':
    app.run(debug=True, port=5000)
