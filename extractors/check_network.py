import requests
response = requests.get('https://www.google.com/')
exit(response.status_code)
