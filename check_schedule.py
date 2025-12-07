import urllib.request
import json

BASE_URL = "http://192.168.2.10:8123/api"
TOKEN = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJjZjRjZmM3NWJlYWY0NTkxOGQ1MDZlYjYwZDI2MDZkNSIsImlhdCI6MTc2NTA1NTE2MCwiZXhwIjoyMDgwNDE1MTYwfQ.fduPBNRuvv_IUt75u0JBVSXM7GfTZyUvvq5YHEJnMU8"
HEADERS = {
    "Authorization": f"Bearer {TOKEN}",
    "Content-Type": "application/json",
}

def get_state(entity_id):
    url = f"{BASE_URL}/states/{entity_id}"
    req = urllib.request.Request(url, headers=HEADERS)
    try:
        with urllib.request.urlopen(req) as response:
            if response.status == 200:
                return json.loads(response.read().decode('utf-8'))
    except Exception as e:
        print(f"[ERROR] {entity_id}: {e}")
    return None

# Check key entities
entities = [
    "switch.gazebo_stove_enable_schedule",
    "select.gazebo_stove_presence_mode",
    "input_datetime.away_mode_time",
]

print("=== Enable Schedule Check ===")
for eid in entities:
    state = get_state(eid)
    if state:
        print(f"[FOUND] {eid}: {state['state']}")
    else:
        print(f"[MISSING] {eid}")

# Also list all gazebo switches
print("\n=== All Gazebo Switches ===")
states_url = f"{BASE_URL}/states"
req = urllib.request.Request(states_url, headers=HEADERS)
try:
    with urllib.request.urlopen(req) as response:
        all_states = json.loads(response.read().decode('utf-8'))
        for s in all_states:
            if 'gazebo' in s['entity_id'].lower() and s['entity_id'].startswith('switch.'):
                print(f"{s['entity_id']}: {s['state']}")
except Exception as e:
    print(f"[ERROR] {e}")
