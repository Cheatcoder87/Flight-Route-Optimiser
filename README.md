# Flight Route Optimiser ✈️

A C++ and Python-based flight route optimizer with a Flask web interface that:
- Finds optimal flight routes based on cost and duration
- Fetches real-time flight status using AeroDataBox API.
- Flight list is generated using the AviationStack API.
- Provides a simple web UI to search and visualize flights.


## 🚀 Tech Stack
- C++ (core routing engine)
- Python (Flask backend)
- HTML/CSS (Frontend)
- AviationStack / AeroDataBox API (Flight status)


## ✅ Features
- Displays real-time flight status
- Uses graph algorithms to find optimal routes
- Generates latest flight between some of the popular airports
- User-friendly interface


## 📌 Setup
```bash
# Setup
pip install -r requirements.txt
# Compile C++ code
g++ code.cpp -o code
# to generate the latest flight list
python3 init.py
# Run Flask
python3 app.py
