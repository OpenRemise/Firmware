import json, os, requests


run_get_requests = True

# Open session
s = requests.Session()

"""
GET
"""
if run_get_requests:
    r = s.get("http://remise.local/sys/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())
