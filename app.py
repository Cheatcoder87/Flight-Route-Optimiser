from pathlib import Path
import subprocess

from flask import Flask, render_template, request,redirect,url_for
import requests

app = Flask(__name__)

@app.route('/', methods=['GET'])
def home():
    return render_template('homepage.html')

@app.route('/index', methods=['POST'])
def index():
    option = request.form['options']
    if option == "find":
        return redirect(url_for('find'))
    elif option == "list":
        return redirect(url_for('list1'))
    elif option == "status":
        return redirect(url_for('status'))
    else:
        return "Invalid option", 400
    

@app.route('/status', methods=['GET', 'POST'])
def status():
    if request.method == 'POST':
        flight_code = request.form['flight_code']
        flight_date = request.form['date']

        url = f"https://aerodatabox.p.rapidapi.com/flights/number/{flight_code}/{flight_date}"
        headers = {
            "X-RapidAPI-Key": "your-api-key",
            "X-RapidAPI-Host": "aerodatabox.p.rapidapi.com"
        }

        try:
            response = requests.get(url, headers=headers)
            data = response.json()

            if isinstance(data, list) and len(data) > 0:
                flight = data[0]

                return render_template("status.html",
                    status=flight.get('status', 'Unknown'),
                    flight_number=flight.get('number'),
                    call_sign=flight.get('callSign'),
                    airline=flight.get('airline', {}).get('name'),
                    aircraft=flight.get('aircraft', {}).get('model'),

                    departure=flight.get('departure', {}).get('airport', {}).get('name'),
                    scheduled_departure=flight.get('departure', {}).get('scheduledTime', {}).get('local'),
                    departure_terminal=flight.get('departure', {}).get('terminal', 'N/A'),

                    arrival=flight.get('arrival', {}).get('airport', {}).get('name'),
                    scheduled_arrival=flight.get('arrival', {}).get('scheduledTime', {}).get('local'),
                    predicted_arrival=flight.get('arrival', {}).get('predictedTime', {}).get('local', 'N/A'),
                    arrival_terminal=flight.get('arrival', {}).get('terminal', 'N/A')
                )
            else:
                return render_template("status.html", error="No flight data found.")
        except Exception as e:
            return render_template("status.html", error=f"Error fetching flight status: {str(e)}")

    return render_template("status.html")



@app.route('/find', methods=['GET', 'POST'])
def find():
    cities=allcities()
    if request.method == 'POST':
        source = request.form['source']
        destination = request.form['destination']
        time = request.form['time']
        factor_b = request.form['factor_b']
        factor_c = request.form['factor_c']

        print("Running subprocess with args:", [
            f"{Path.cwd()}/code",
            'find',
            source,
            destination,
            time,
            factor_b,
            factor_c,
        ], flush=True)

        try:
            result = subprocess.run(
                [
                    f"{Path.cwd()}/code",
                    'find',
                    source,
                    destination,
                    time,
                    factor_b,
                    factor_c,
                ],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                timeout=10
            )

            print("Subprocess completed", flush=True)
            print("Stdout:", result.stdout, flush=True)
            print("Stderr:", result.stderr, flush=True)

            if result.returncode != 0:
                return f"Error from executable: {result.stderr}"

            flights, summary = parse_output(result.stdout)

            if flights is None:
                return render_template('result.html', error=summary)

            return render_template('result.html', options=flights)

        except subprocess.TimeoutExpired:
            return "Error: The route finder process timed out."
        except Exception as e:
            return f"Exception: {str(e)}"
    
    return render_template('find.html', cities=sorted(cities))



@app.route('/list1',methods=['GET','POST'])
def list1():
    cities=allcities()
    try:
        result = subprocess.run(
            [f"{Path.cwd()}/code", 'list'],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            timeout=10
        )

        if result.returncode != 0:
            return f"Error from executable: {result.stderr}"

        flights = parse_list_output(result.stdout)
        return render_template("list.html", flights=flights,cities=cities)

    except subprocess.TimeoutExpired:
        return "Error: The route finder process timed out."
    except Exception as e:
        return f"Exception: {str(e)}"

def parse_output(output):
    if "No routes found" in output:
        return None, "Sorry, no routes found."

    all_options = []
    current_option = []
    total_cost = ""
    total_time = ""

    lines = output.strip().splitlines()
    i = 0

    while i < len(lines):
        line = lines[i].strip()

        if line.startswith("Option"):
            if current_option:
                all_options.append({
                    'flights': current_option,
                    'total_cost': total_cost,
                    'total_time': total_time
                })
                current_option = []
            i += 1

        elif "->" in line and (i + 1 < len(lines) and lines[i + 1].startswith("Flight Code:")):
            route = line.strip()
            i += 1
            flight_code = lines[i].replace("Flight Code:", "").strip()
            i += 1
            flight_name = lines[i].replace("Flight:", "").strip()
            i += 1

            flight = {
                'code': flight_code,
                'name': flight_name,
                'route': route,
                'departure': '',
                'duration': '',
                'cost': '',
                'layover': None
            }

            while i < len(lines):
                l = lines[i].strip()
                if l.startswith("Departure:"):
                    flight['departure'] = l.replace("Departure:", "").strip()
                elif l.startswith("Duration:"):
                    flight['duration'] = l.replace("Duration:", "").replace("min", "").strip()
                elif l.startswith("Cost:"):
                    flight['cost'] = l.replace("Cost:", "").strip()
                elif l.startswith("Layover Time:"):
                    flight['layover'] = l.replace("Layover Time:", "").strip()
                elif l.startswith("Option") or "->" in l or l.startswith("Total"):
                    break
                i += 1

            current_option.append(flight)

        elif line.startswith("Total Cost of Trip:"):
            total_cost = line.replace("Total Cost of Trip:", "").strip()
            i += 1

        elif line.startswith("Total Time of Trip:"):
            total_time = line.replace("Total Time of Trip:", "").strip()
            i += 1

        else:
            i += 1

    if current_option:
        all_options.append({
            'flights': current_option,
            'total_cost': total_cost,
            'total_time': total_time
        })

    return all_options, None


def parse_list_output(output):
    flights = []
    lines = output.strip().splitlines()

    for line in lines:
        if not line.strip() or line.startswith("Program") or line.startswith("argc") or line.startswith("argv"):
            continue 

        parts = line.strip().split()
        if len(parts) < 8:
            continue

        from_airport = parts[0]
        to_airport = parts[1]
        distance = parts[2]
        cost = parts[3]
        duration = parts[4]
        departure = parts[5]
        name = parts[6].replace('_',' ')
        code = parts[7]

        flights.append({
            'from': from_airport,
            'to': to_airport,
            'distance': distance,
            'cost': cost,
            'time': duration,
            'departure': departure,
            'name': name,
            'code': code,
        })

    return flights

def allcities():
    cities=set()
    try:
        with open("flight.txt", "r") as f:
            for line in f:
                if line.strip() == "" or line.startswith("Program") or line.startswith("argc"):
                    continue
                parts = line.strip().split()
                if len(parts) < 7:
                    continue
                source = parts[0]
                to = parts[1]
                if '_' in source:
                    parts = source.split('_')
                    source = parts[0]  + parts[1] 

                if '_' in to:
                    parts = to.split('_')
                    to = parts[0]  + parts[1] 

                cities.add(source)
                cities.add(to)
    except Exception as e:
        print(f"Error reading flights.txt: {e}")
    return cities

if __name__ == "__main__":
    app.run(debug=True)
