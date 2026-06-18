import requests
import sys

API_KEY = "e220cc79215d4f44bf18a994ea99c5bb"

if len(sys.argv) < 2:
    print("Usage: news [technology|sports|business|health]")
    exit()

category = sys.argv[1]

url = (
    f"https://newsapi.org/v2/top-headlines"
    f"?country=us"
    f"&category={category}"
    f"&apiKey={API_KEY}"
)

response = requests.get(url)
data = response.json()

if data["status"] != "ok":
    print("Error:", data)
    exit()

print("\n===== TOP NEWS =====\n")

for i, article in enumerate(data["articles"][:5], start=1):
    print(f"{i}. {article['title']}")