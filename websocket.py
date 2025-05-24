import asyncio
import websockets
import re
import json

connected_clients = set()

async def websocket_handler(websocket):
    print(f"Client connected: {websocket.remote_address}")
    connected_clients.add(websocket)
    try:
        async for message in websocket:
    
            # Try to parse JSON message first
            try:
                data = json.loads(message)
                humidity = data.get("humidity")
                temperature = data.get("temperature")
                print(f"[JSON] Humidity: {humidity}% Temperature: {temperature}°C")
            except Exception:
                # Fallback: parse ESP_LOGI string
                match = re.search(r"Humidity: ([\d.]+)% Temperature: ([\d.]+)°C", message)
                if match:
                    humidity = float(match.group(1))
                    temperature = float(match.group(2))
                    print(f"[LOG] Humidity: {humidity}% Temperature: {temperature}°C")
                    data = {"humidity": humidity, "temperature": temperature}
                else:
                    data = None
            # Broadcast to all other connected clients (e.g., JS clients)
            if 'data' in locals() and data is not None:
                msg = json.dumps(data)
                await asyncio.gather(*[
                    client.send(msg) for client in connected_clients if client != websocket
                ])
    except websockets.ConnectionClosed:
        print(f"Client disconnected: {websocket.remote_address}")
    finally:
        connected_clients.remove(websocket)

async def main():
    server = await websockets.serve(websocket_handler, "0.0.0.0", 8765)
    print("WebSocket server started on ws://0.0.0.0:8765")
    await server.wait_closed()

if __name__ == "__main__":
    asyncio.run(main())