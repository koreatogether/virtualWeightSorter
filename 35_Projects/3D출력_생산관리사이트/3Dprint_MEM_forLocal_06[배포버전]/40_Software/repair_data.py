
import json
import os

DB_PATH = r'E:\project\35_Projects\3D출력_생산관리사이트\3Dprint_MEM_forLocal_03\40_Software\data\db.json'

def repair_db():
    if not os.path.exists(DB_PATH):
        print(f"File not found: {DB_PATH}")
        return

    with open(DB_PATH, 'r', encoding='utf-8') as f:
        data = json.load(f)

    inventory_map = {item['id']: item for item in data.get('inventory', [])}
    
    orders_fixed = 0
    for order in data.get('orders', []):
        mat_val = order.get('material', '')
        # Check if the material value is actually an inventory ID
        if mat_val in inventory_map:
            inv_item = inventory_map[mat_val]
            print(f"Fixing Order {order.get('id')}: Material ID '{mat_val}' -> Name '{inv_item['material']}', Batch '{inv_item.get('batch', '')}'")
            
            order['material'] = inv_item['material']
            order['batch'] = inv_item.get('batch', '')
            orders_fixed += 1

    if orders_fixed > 0:
        with open(DB_PATH, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        print(f"Successfully repaired {orders_fixed} orders.")
    else:
        print("No orders needed repair.")

if __name__ == "__main__":
    repair_db()
