import requests
import time
import random

send_interval = 60  

# Define the remote server URL
server_url = "http://178.192.219.78:8080/sensor-data"  # Replace with your Flask server URL

# Function to generate random sensor data 
def generate_sensor_data(sensor_id):
    return {
        "temperature": round(random.uniform(15, 35), 2),
        "humidity": random.randint(20, 80),
        "light": random.randint(0, 2000),
        "id": sensor_id
    }

# Function to simulate sensor POST request
def send_post_request( interval):
    while True:
        sensor_id = random.randrange(1,20) 
        data = generate_sensor_data(sensor_id)
        print(f"Sending data: {data}")

        try:
            response = requests.post(server_url, json=data)
            if response.status_code == 200:
                print("Data successfully sent")
            else:
                print(f"Failed to send data. Status code: {response.status_code}")
        except Exception as e:
            print(f"Error sending data: {e}")

        # Wait for the specified interval before sending the next data
        time.sleep(interval)


if __name__ == "__main__":
    send_post_request( send_interval)
