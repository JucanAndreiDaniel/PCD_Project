import requests

for i in range(0,10000):
    url = "http://localhost:7437/upload"

    payload={}
    files=[
    ('',('index.html',open('/home/andrew/faculta/PCD/PCD_Project/ServerUlfus/static/index.html','rb'),'text/html'))
    ]
    headers = {}
    print(f"ceva {i}")

    response = requests.request("POST", url, headers=headers, data=payload, files=files)

print(response.text)