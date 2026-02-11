from flask import Flask, render_template, jsonify, request
import html
import json
import os
import uuid
from datetime import datetime, timedelta

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_FILE = os.path.join(BASE_DIR, 'data', 'db.json')
ACTION_LOG_FILE = os.path.join(BASE_DIR, 'data', 'user_actions.log')

ALLOW_REMOTE = os.environ.get('ALLOW_REMOTE', 'false').lower() == 'true'
REQUIRE_API_KEY = os.environ.get('REQUIRE_API_KEY', 'false').lower() == 'true'
API_KEY = os.environ.get('API_KEY', '')


def _is_local_request():
    ip = (request.remote_addr or '').strip()
    return ip in ('127.0.0.1', '::1', 'localhost', '')


def sanitize_text(value, max_len=120):
    if value is None:
        return ''
    text = html.escape(str(value).strip(), quote=True)
    return text[:max_len]


def parse_float(value, field_name, min_value=0.0):
    try:
        number = float(value)
    except (TypeError, ValueError):
        raise ValueError(f"Invalid number for '{field_name}'")
    if number < min_value:
        raise ValueError(f"'{field_name}' must be >= {min_value}")
    return number


def get_json_payload(required_keys=None):
    payload = request.get_json(silent=True)
    if not isinstance(payload, dict):
        raise ValueError('Invalid JSON body')
    required_keys = required_keys or []
    missing = [k for k in required_keys if k not in payload]
    if missing:
        raise ValueError(f"Missing required fields: {', '.join(missing)}")
    return payload


def log_action(action_type, target, details):
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    ip = request.remote_addr
    safe_details = sanitize_text(details, 400)
    log_entry = f"[{timestamp}] [IP: {ip}] [ACTION: {action_type}] [TARGET: {target}] - {safe_details}\n"
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
            if 'orders' not in data:
                data['orders'] = []
            if 'schedules' not in data:
                data['schedules'] = []
            if 'inventory' not in data:
                data['inventory'] = []
            if 'printers' not in data:
                data['printers'] = []
            return data
        except json.JSONDecodeError:
            return {"orders": [], "schedules": [], "inventory": [], "printers": []}


def write_db(data):
    with open(DB_FILE, 'w', encoding='utf-8') as f:
        json.dump(data, f, indent=2, ensure_ascii=False)


def check_conflict(schedules, printer_id, date_str):
    target_date = date_str.split('T')[0]
    for item in schedules:
        if item.get('printer_id') == printer_id:
            s_date = item.get('start_time', '').split('T')[0]
            if s_date == target_date:
                return True
    return False


@app.before_request
def security_gate():
    if not ALLOW_REMOTE and not _is_local_request():
        return jsonify({"status": "error", "message": "Remote access is disabled"}), 403

    if REQUIRE_API_KEY and request.path.startswith('/api/v1'):
        if request.headers.get('X-API-Key', '') != API_KEY:
            return jsonify({"status": "error", "message": "Invalid API key"}), 401

    if request.path.startswith('/api/v1') and request.method in ('POST', 'PATCH', 'DELETE'):
        if request.headers.get('X-Requested-With', '') != 'XMLHttpRequest':
            return jsonify({"status": "error", "message": "Invalid request header"}), 400


@app.after_request
def set_security_headers(response):
    response.headers['X-Content-Type-Options'] = 'nosniff'
    response.headers['X-Frame-Options'] = 'DENY'
    response.headers['Referrer-Policy'] = 'same-origin'
    response.headers['Cache-Control'] = 'no-store'
    response.headers['Content-Security-Policy'] = "default-src 'self'; script-src 'self' https://cdn.jsdelivr.net; style-src 'self' 'unsafe-inline' https://cdn.jsdelivr.net; img-src 'self' data:; connect-src 'self';"
    return response


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/api/v1/data', methods=['GET'])
def get_data():
    return jsonify(read_db())


@app.route('/api/v1/orders', methods=['POST'])
def add_order():
    try:
        req = get_json_payload(['product_name', 'target_quantity'])
        data = read_db()

        new_order = {
            'id': str(uuid.uuid4())[:8],
            'product_name': sanitize_text(req.get('product_name'), 120),
            'target_quantity': parse_float(req.get('target_quantity'), 'target_quantity'),
            'initial_stock': parse_float(req.get('initial_stock', 0), 'initial_stock'),
            'completed_quantity': 0.0,
            'status': 'in_progress',
            'material': sanitize_text(req.get('material', 'PLA'), 40),
            'color': sanitize_text(req.get('color', 'Black'), 40),
            'unit_weight_g': parse_float(req.get('unit_weight_g', 0), 'unit_weight_g'),
            'deadline': sanitize_text(req.get('deadline', ''), 20),
            'created_at': datetime.now().isoformat()
        }

        data['orders'].append(new_order)
        write_db(data)
        log_action('CREATE_ORDER', new_order['id'], f"Product: {new_order['product_name']}, Target: {new_order['target_quantity']}")
        return jsonify({"status": "success", "id": new_order['id']}), 201
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/orders/<id>', methods=['PATCH'])
def update_order(id):
    try:
        req_data = get_json_payload()
        data = read_db()
        for order in data['orders']:
            if order['id'] == id:
                if 'product_name' in req_data:
                    order['product_name'] = sanitize_text(req_data['product_name'], 120)
                if 'target_quantity' in req_data:
                    order['target_quantity'] = parse_float(req_data['target_quantity'], 'target_quantity')
                if 'initial_stock' in req_data:
                    order['initial_stock'] = parse_float(req_data['initial_stock'], 'initial_stock')
                if 'deadline' in req_data:
                    order['deadline'] = sanitize_text(req_data['deadline'], 20)
                if 'material' in req_data:
                    order['material'] = sanitize_text(req_data['material'], 40)
                if 'color' in req_data:
                    order['color'] = sanitize_text(req_data['color'], 40)
                if 'unit_weight_g' in req_data:
                    order['unit_weight_g'] = parse_float(req_data['unit_weight_g'], 'unit_weight_g')

                actual_sum = sum(float(s.get('actual_quantity', 0)) for s in data['schedules'] if s.get('order_id') == id)
                order['status'] = 'completed' if (actual_sum + float(order.get('initial_stock', 0))) >= float(order['target_quantity']) else 'in_progress'

                write_db(data)
                log_action('UPDATE_ORDER', id, f"Updated fields: {list(req_data.keys())}")
                return jsonify({"status": "success"})
        return jsonify({"status": "error", "message": "Order not found"}), 404
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/orders/<id>', methods=['DELETE'])
def delete_order(id):
    data = read_db()
    delete_schedules = request.args.get('delete_schedules', 'false').lower() == 'true'
    data['orders'] = [o for o in data['orders'] if o['id'] != id]
    if delete_schedules:
        data['schedules'] = [s for s in data['schedules'] if s.get('order_id') != id]
    write_db(data)
    log_action('DELETE_ORDER', id, f"Order removed (Schedules deleted: {delete_schedules})")
    return jsonify({"status": "success"})


@app.route('/api/v1/schedules/auto', methods=['POST'])
def auto_schedule():
    try:
        req = get_json_payload(['order_id', 'printer_id', 'daily_qty', 'start_date'])
        data = read_db()
        order_id = sanitize_text(req.get('order_id'), 20)
        printer_id = sanitize_text(req.get('printer_id'), 20)
        daily_qty = parse_float(req.get('daily_qty'), 'daily_qty', min_value=0.0001)
        start_date_str = sanitize_text(req.get('start_date'), 30)

        target_order = next((o for o in data['orders'] if o['id'] == order_id), None)
        if not target_order:
            return jsonify({"status": "error", "message": "Order not found"}), 404

        remaining_qty = float(target_order['target_quantity']) - float(target_order.get('initial_stock', 0)) - float(target_order.get('completed_quantity', 0))
        if remaining_qty <= 0:
            return jsonify({"status": "error", "message": "Order already completed"}), 400

        created_schedules = []
        conflicts = []
        current_date = datetime.strptime(start_date_str.split('T')[0], '%Y-%m-%d')

        while remaining_qty > 0:
            plan_qty = min(remaining_qty, daily_qty)
            current_date_str = current_date.strftime('%Y-%m-%dT09:00')
            is_conflict = check_conflict(data['schedules'], printer_id, current_date_str) or check_conflict(created_schedules, printer_id, current_date_str)
            new_task = {
                "id": str(uuid.uuid4())[:8],
                "order_id": order_id,
                "printer_id": printer_id,
                "planned_quantity": plan_qty,
                "actual_quantity": 0.0,
                "status": "pending",
                "start_time": current_date_str,
                "created_at": datetime.now().isoformat(),
                "conflict": is_conflict
            }
            if is_conflict:
                conflicts.append(current_date_str.split('T')[0])
            data['schedules'].append(new_task)
            created_schedules.append(new_task)
            remaining_qty -= plan_qty
            current_date += timedelta(days=1)

        write_db(data)
        log_action('AUTO_SCHEDULE', order_id, f"Created {len(created_schedules)} days for printer {printer_id}")
        return jsonify({"status": "success", "count": len(created_schedules), "conflicts": conflicts}), 201
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/schedules', methods=['POST'])
def add_schedule():
    try:
        req = get_json_payload(['order_id', 'printer_id', 'planned_quantity'])
        data = read_db()
        new_task = {
            'id': str(uuid.uuid4())[:8],
            'order_id': sanitize_text(req.get('order_id'), 20),
            'printer_id': sanitize_text(req.get('printer_id'), 20),
            'planned_quantity': parse_float(req.get('planned_quantity'), 'planned_quantity', min_value=0.0001),
            'actual_quantity': 0.0,
            'status': 'pending',
            'start_time': sanitize_text(req.get('start_time', datetime.now().strftime('%Y-%m-%dT%H:%M')), 30),
            'created_at': datetime.now().isoformat()
        }
        new_task['conflict'] = check_conflict(data['schedules'], new_task['printer_id'], new_task['start_time'])
        data['schedules'].append(new_task)
        write_db(data)
        log_action('ADD_SCHEDULE', new_task['id'], f"Order: {new_task.get('order_id')}, Printer: {new_task.get('printer_id')}")
        return jsonify({"status": "success", "id": new_task['id'], "conflict": new_task['conflict']}), 201
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/schedules/<id>', methods=['PATCH'])
def update_schedule(id):
    try:
        req_data = get_json_payload()
        data = read_db()
        target_schedule = None

        for item in data['schedules']:
            if item['id'] == id:
                target_schedule = item
                if 'actual_quantity' in req_data:
                    added_qty = parse_float(req_data['actual_quantity'], 'actual_quantity')
                    old_actual = float(item.get('actual_quantity', 0))
                    new_actual = old_actual + added_qty
                    item['actual_quantity'] = new_actual

                    planned = float(item.get('planned_quantity', 0))
                    old_excess = max(0.0, old_actual - planned)
                    new_excess = max(0.0, new_actual - planned)
                    delta_excess = new_excess - old_excess

                    if delta_excess > 0:
                        order_id = item.get('order_id')
                        current_start_time = item.get('start_time', '')
                        future_schedules = sorted(
                            [s for s in data['schedules'] if s.get('order_id') == order_id and s.get('status') == 'pending' and s['id'] != id and s.get('start_time', '') > current_start_time],
                            key=lambda x: x.get('start_time', '')
                        )
                        for fs in future_schedules:
                            if delta_excess <= 0:
                                break
                            fs_planned = float(fs.get('planned_quantity', 0))
                            can_reduce = min(fs_planned, delta_excess)
                            fs['planned_quantity'] = fs_planned - can_reduce
                            delta_excess -= can_reduce

                for key, value in req_data.items():
                    if key != 'actual_quantity':
                        item[key] = sanitize_text(value, 60) if isinstance(value, str) else value
                break

        if not target_schedule:
            return jsonify({"status": "error", "message": "Not found"}), 404

        order_id = target_schedule.get('order_id')
        order_info = None
        if order_id:
            total_completed = sum(float(s.get('actual_quantity', 0)) for s in data['schedules'] if s.get('order_id') == order_id)
            for order in data['orders']:
                if order['id'] == order_id:
                    order['completed_quantity'] = total_completed
                    order['status'] = 'completed' if (order['completed_quantity'] + float(order.get('initial_stock', 0))) >= float(order['target_quantity']) else 'in_progress'
                    order_info = {
                        "id": order['id'],
                        "status": order['status'],
                        "material": order.get('material'),
                        "color": order.get('color'),
                        "product_name": order.get('product_name')
                    }
                    break

        write_db(data)
        log_action('UPDATE_SCHEDULE_ACTUAL', id, f"New actual total: {target_schedule.get('actual_quantity')}")
        return jsonify({"status": "success", "new_actual": target_schedule.get('actual_quantity'), "order_info": order_info})
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/schedules/<id>', methods=['DELETE'])
def delete_schedule(id):
    data = read_db()
    order_id = None
    for schedule in data['schedules']:
        if schedule['id'] == id:
            order_id = schedule.get('order_id')
            break

    if not order_id:
        return jsonify({"status": "error", "message": "Not found"}), 404

    data['schedules'] = [s for s in data['schedules'] if s['id'] != id]
    total_completed = sum(float(s.get('actual_quantity', 0)) for s in data['schedules'] if s.get('order_id') == order_id)
    for order in data['orders']:
        if order['id'] == order_id:
            order['completed_quantity'] = total_completed
            order['status'] = 'completed' if (order['completed_quantity'] + float(order.get('initial_stock', 0))) >= float(order['target_quantity']) else 'in_progress'
            break

    write_db(data)
    log_action('DELETE_SCHEDULE', id, 'Schedule removed')
    return jsonify({"status": "success"})


@app.route('/api/v1/inventory', methods=['POST'])
def add_inventory():
    try:
        req = get_json_payload(['material', 'color', 'remaining_weight_g'])
        data = read_db()
        new_inv = {
            'id': str(uuid.uuid4())[:8],
            'material': sanitize_text(req.get('material'), 40),
            'color': sanitize_text(req.get('color'), 40),
            'batch': sanitize_text(req.get('batch', ''), 20),
            'remaining_weight_g': parse_float(req.get('remaining_weight_g'), 'remaining_weight_g'),
            'updated_at': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        }
        data['inventory'].append(new_inv)
        write_db(data)
        log_action('ADD_INVENTORY', new_inv['id'], f"Material: {new_inv['material']}, Batch: {new_inv['batch']}, Weight: {new_inv['remaining_weight_g']}g")
        return jsonify({"status": "success", "id": new_inv['id']}), 201
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/inventory/<id>', methods=['PATCH'])
def update_inventory(id):
    try:
        req_data = get_json_payload()
        data = read_db()
        for item in data['inventory']:
            if item['id'] == id:
                if 'material' in req_data:
                    item['material'] = sanitize_text(req_data['material'], 40)
                if 'color' in req_data:
                    item['color'] = sanitize_text(req_data['color'], 40)
                if 'batch' in req_data:
                    item['batch'] = sanitize_text(req_data['batch'], 20)
                if 'remaining_weight_g' in req_data:
                    item['remaining_weight_g'] = parse_float(req_data['remaining_weight_g'], 'remaining_weight_g')
                item['updated_at'] = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                write_db(data)
                log_action('UPDATE_INVENTORY', id, f"Updated fields: {list(req_data.keys())}")
                return jsonify({"status": "success"})
        return jsonify({"status": "error", "message": "Inventory not found"}), 404
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/inventory/<id>', methods=['DELETE'])
def delete_inventory(id):
    data = read_db()
    data['inventory'] = [i for i in data['inventory'] if i['id'] != id]
    write_db(data)
    log_action('DELETE_INVENTORY', id, 'Inventory removed')
    return jsonify({"status": "success"})


@app.route('/api/v1/printers', methods=['POST'])
def add_printer():
    try:
        req = get_json_payload(['model'])
        data = read_db()
        model = sanitize_text(req.get('model', 'PRN'), 60)
        asset_id = sanitize_text(req.get('asset_id', ''), 60) or f"{model}-{str(uuid.uuid4())[:4]}"
        purchase_date = sanitize_text(req.get('purchase_date', datetime.now().strftime('%Y-%m-%d')), 20)

        new_printer = {
            'id': str(uuid.uuid4())[:8],
            'model': model,
            'asset_id': asset_id,
            'purchase_date': purchase_date,
            'status': 'idle'
        }
        data['printers'].append(new_printer)
        write_db(data)
        log_action('ADD_PRINTER', new_printer['id'], f"Model: {new_printer['model']}, AssetID: {new_printer['asset_id']}")
        return jsonify({"status": "success", "id": new_printer['id']}), 201
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/printers/<id>', methods=['PATCH'])
def update_printer(id):
    try:
        req_data = get_json_payload()
        data = read_db()
        for item in data['printers']:
            if item['id'] == id:
                if 'model' in req_data:
                    item['model'] = sanitize_text(req_data['model'], 60)
                if 'asset_id' in req_data:
                    item['asset_id'] = sanitize_text(req_data['asset_id'], 60)
                if 'purchase_date' in req_data:
                    item['purchase_date'] = sanitize_text(req_data['purchase_date'], 20)
                write_db(data)
                log_action('UPDATE_PRINTER', id, f"Updated fields: {list(req_data.keys())}")
                return jsonify({"status": "success"})
        return jsonify({"status": "error", "message": "Printer not found"}), 404
    except ValueError as e:
        return jsonify({"status": "error", "message": str(e)}), 400


@app.route('/api/v1/printers/<id>', methods=['DELETE'])
def delete_printer(id):
    data = read_db()
    data['printers'] = [p for p in data['printers'] if p['id'] != id]
    write_db(data)
    log_action('DELETE_PRINTER', id, 'Printer removed')
    return jsonify({"status": "success"})


@app.route('/api/v1/printers/<id>/busy_dates', methods=['GET'])
def get_printer_busy_dates(id):
    data = read_db()
    busy_dates = set()
    for schedule in data['schedules']:
        if schedule.get('printer_id') == id and schedule.get('status') != 'completed':
            date_str = schedule.get('start_time', '').split('T')[0]
            if date_str:
                busy_dates.add(date_str)
    return jsonify(list(busy_dates))


if __name__ == '__main__':
    app.run(host='127.0.0.1', debug=False, port=5000)
