import requests
from datetime import datetime, timezone
import time
import re

API_KEY = "your-api-key"

BASE_URL = "http://api.aviationstack.com/v1/flights"

popular_airports = [
    "DEL", 
    "BOM", 
    "BLR",  
    "HYD", 
    "CCU", 
    "MAA",
    "AMD",  
    "GOI",  
    "PNQ", 
]

iata_to_city = {
    "DEL": "Delhi(DEL)",
    "BOM": "Mumbai(BOM)",
    "BLR": "Bangalore(BLR)",
    "HYD": "Hyderabad(HYD)",
    "CCU": "Kolkata(CCU)",
    "MAA": "Chennai(MAA)",
    "AMD": "Ahmedabad(AMD)",
    "GOI": "Goa(GOI)",
    "PNQ": "Pune(PNQ)",
}

def get_distance(from_iata, to_iata):
    dummy_distances = {
        ("DEL", "BOM"): 1150,
        ("DEL", "BLR"): 1700,
        ("DEL", "CCU"): 1300,
        ("BLR", "BOM"): 840,
        ("HYD", "DEL"): 1250,
        ("MAA", "DEL"): 1750,
        ("BOM", "CCU"): 1650,
        ("BLR", "CCU"): 1550,
    }
    return dummy_distances.get((from_iata, to_iata), 1000)

def fetch_flights(from_iata, to_iata):
    params = {
        "access_key": API_KEY,
        "dep_iata": from_iata,
        "arr_iata": to_iata,
        "limit": 100
    }

    try:
        res = requests.get(BASE_URL, params=params)
        data = res.json()
        flights_data = data.get("data", [])
        print(f"🛰️ {from_iata} → {to_iata} | Flights found: {len(flights_data)}")

        flights = []

        for f in flights_data:
            dep_time = f.get("departure", {}).get("scheduled")
            arr_time = f.get("arrival", {}).get("scheduled")
            if not dep_time or not arr_time:
                continue

            try:
                dep_dt = datetime.fromisoformat(dep_time)
                arr_dt = datetime.fromisoformat(arr_time)
                travel_minutes = int((arr_dt - dep_dt).total_seconds() / 60)
            except:
                travel_minutes = -1

            distance = get_distance(from_iata, to_iata)
            cost = distance * 6

            raw_airline = f.get("airline", {}).get("name", "UnknownAirline")
            raw_number = f.get("flight", {}).get("number", "UNK000")

            clean_airline = re.sub(r'[^a-zA-Z0-9]', '', raw_airline)
            clean_number = re.sub(r'[^a-zA-Z0-9]', '', raw_number)
            airline_code = f"{clean_airline}_{clean_number}"

            from_city = iata_to_city.get(from_iata, from_iata)
            to_city = iata_to_city.get(to_iata, to_iata)

            flight_str = f"{from_city} {to_city} {travel_minutes} {cost} {distance} {dep_dt.strftime('%H:%M')} {airline_code}"
            flights.append(flight_str)

        return flights

    except Exception as e:
        print(f"❌ Error fetching {from_iata} → {to_iata}:", e)
        return []

def build_flight_file():
    filename = "flight.txt"
    seen = set()

    with open(filename, "w") as f:
        for i in range(len(popular_airports)): 
            for j in range(len(popular_airports)):
                if i == j:
                    continue

                from_iata = popular_airports[i]
                to_iata = popular_airports[j]
                key = (from_iata, to_iata)
                if key in seen:
                    continue
                seen.add(key)

                print(f"Fetching {from_iata} → {to_iata}...")
                flights = fetch_flights(from_iata, to_iata)

                for flight in flights:
                    f.write(flight + "\n")

                time.sleep(1.2)

    print("✅ All flights saved to flight.txt")

if __name__ == "__main__":
    build_flight_file()
