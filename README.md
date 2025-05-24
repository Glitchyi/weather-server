| Supported Targets | ESP32 | 
| ----------------- | ----- | 

# ESP32 Weather Server & Climate Prediction

This project is a simple solution for collecting local climate data using an ESP32 and sensors, and providing real-time weather information and predictions through a web interface and an API.

## Tech Stack
- **ESP-IDF (Espressif SDK):** Used for developing the ESP32 firmware that reads sensor data and communicates with the server.
- **Python:** Powers the WebSocket server for real-time data relay between the ESP32 and web clients.
- **HTML & Tailwind CSS:** The current web interface is built with plain HTML and Tailwind for rapid prototyping and modern UI.
- **Svelte (Planned):** Future versions will migrate the frontend to Svelte for a more dynamic, maintainable, and scalable web application.
- **PyTorch (Planned):** Will be used for developing and running climate prediction models based on collected sensor data.

## Getting Started
1. **Run the WebSocket Server:**
   ```bash
   python3 websocket.py
   ```
   The server listens on `ws://<host>:8765` for incoming sensor data and client connections.

2. **Open the Web Interface:**
   Open `frontend/index.html` in your browser. The interface connects to the WebSocket server and displays live sensor data.

3. **Connect Your ESP32:**
   Configure your ESP32 to send humidity and temperature data (as JSON or log strings) to the WebSocket server.

## Project Structure
- `websocket.py` — Python WebSocket server for relaying sensor data.
- `frontend/index.html` — Modern web client for real-time weather display.
- `main/` — ESP32 firmware source (C, using ESP-IDF).

## Roadmap
- [x] Real-time sensor data display
- [ ] Climate prediction algorithms
- [ ] REST API for data and predictions
- [ ] Historical data visualization

## Requirements
- Python 3.7+
- ESP32 with DHT or similar sensors
- Modern web browser

---

*This project is under active development. Contributions and suggestions are welcome!*
