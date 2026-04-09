import os
import subprocess
import re
from flask import Flask, render_template, request, redirect, url_for, flash

app = Flask(__name__)
app.secret_key = 'bus-secret-key'

EXE = "./bus_booking"

def run_cmd(args):
    """Runs the C++ executable and captures BOTH stdout and stderr."""
    try:
        result = subprocess.run(
            [EXE] + args, 
            capture_output=True, 
            text=True, 
            check=False
        )
        return (result.stdout + result.stderr).strip()
    except Exception as e:
        print(f"Subprocess Error: {e}")
        return f"System Error: {str(e)}"

def parse_table_line(line):
    """Helper to clean up the C++ setw() whitespace output into a list of words."""
    return re.split(r'\s{2,}', line.strip())

@app.route("/")
def index():
    # Process Routes 
    out_routes = run_cmd(["list_routes"])
    routes = []
    if out_routes:
        for line in out_routes.split("\n"):
            if any(x in line for x in ["ID", "---", "Available"]) or not line.strip():
                continue
            # Inside @app.route("/")
            parts = re.split(r'\s{2,}', line.strip())
            if len(parts) >= 5:
                seat_info = parts[4].split('/')
                booked_count = int(seat_info[0])
                total_capacity = int(seat_info[1])
                
                routes.append({
                    'id': parts[0],
                    'from': parts[1],
                    'to': parts[2],
                    'fare': parts[3],
                    'booked': booked_count, 
                    'cap': total_capacity
                })
    
    # Process Tickets 
    out_tickets = run_cmd(["list_tickets"])
    tickets = []
    if out_tickets:
        for line in out_tickets.split("\n"):
            if any(x in line for x in ["TID", "---", "All Booked"]) or not line.strip():
                continue
            parts = re.split(r'\s{2,}', line.strip())
            if len(parts) >= 5:
                tickets.append({
                    'id': parts[0],
                    'name': parts[1],
                    'rid': parts[2],
                    'seats': parts[3],
                    'fare': parts[4].replace("₹", "").strip()
                })
                
    # Process Stats 
    out_stats = run_cmd(["stats"])
    stats = []
    popular = None
    if out_stats:
        for line in out_stats.split("\n"):
            # Extract Popular Route ID
            if "Most Popular Route: ID" in line:
                match = re.search(r'ID (\d+)', line)
                if match: popular = match.group(1)
            
            # Parse line: Route 1 (From -> To): Tickets: 2 | Revenue: ₹13840.00
            if "Route" in line and "Revenue:" in line:
                try:
                    # Extract Route Name/ID
                    route_name = line.split(':')[0].replace("Route ", "")
                    # Extract Bookings
                    b_match = re.search(r'Tickets: (\d+)', line)
                    # Extract Revenue
                    r_match = re.search(r'Revenue: ₹?([\d.]+)', line)
                    
                    stats.append({
                        'route': route_name,
                        'bookings': b_match.group(1) if b_match else "0",
                        'rev': r_match.group(1) if r_match else "0.00"
                    })
                except Exception: continue
            
    return render_template("index.html", routes=routes, tickets=tickets, stats=stats, popular=popular)

@app.route("/book", methods=["POST"])
def book():
    name = request.form.get("name")
    rid = request.form.get("rid")
    raw_seats = request.form.get("seats")
    seats = [s.strip() for s in raw_seats.split(",") if s.strip()]
    
    # Call: ./bus_system book "Name" <rid> <s1> <s2> ...
    out = run_cmd(["book", name, rid] + seats)
    
    if "Successfully Booked" in out:
        flash(out.split("\n")[-1]) 
    else:
        flash(f"Booking Failed: {out}")
    
    return redirect(url_for("index"))

@app.route("/cancel/<tid>") 
def cancel(tid):
    out = run_cmd(["cancel", str(tid)])
    if "Cancelled Successfully" in out:
        flash(f"Ticket {tid} cancelled.")
    else:
        flash(f"Error: {out}")
    return redirect(url_for("index"))

if __name__ == "__main__":
    if not os.path.exists(EXE):
        print(f"WARNING: {EXE} not found. Please compile your C++ code first!")
    
    app.run(host="0.0.0.0", port=5000, debug=True)