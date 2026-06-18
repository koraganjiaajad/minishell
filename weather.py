import requests
import sys

API_KEY = "62ae505a1e27798730f2761f7b8ed1c9"

if len(sys.argv) < 2:
    print("Usage: weather <city>")
    exit()

# Supports multi-word cities like "New York"
city = " ".join(sys.argv[1:])

url = (
    f"https://api.openweathermap.org/data/2.5/weather"
    f"?q={city}&appid={API_KEY}&units=metric"
)

try:
    response = requests.get(url, timeout=10)
    data = response.json()

    if response.status_code != 200:
        print("API Error:")
        print(data)
        exit()

    print("\n===== Weather Report =====")
    print("City       :", data["name"])
    print("Country    :", data["sys"]["country"])
    print("Temperature:", data["main"]["temp"], "°C")
    print("Feels Like :", data["main"]["feels_like"], "°C")
    print("Humidity   :", data["main"]["humidity"], "%")
    print("Condition  :", data["weather"][0]["description"])
    print("==========================")

except requests.exceptions.RequestException as e:
    print("Network Error:", e)