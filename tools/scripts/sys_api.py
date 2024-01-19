import json, os, requests


run_get_requests = True
run_post_requests = True

valid = '{"state":"Suspended"}'
invalid = '{"state":: '

# Open session
s = requests.Session()

"""
GET
"""
if run_get_requests:
    r = s.get("http://wulf.local/sys/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

"""
POST
"""
if run_post_requests:
    r = s.post("http://wulf.local/sys/", data=valid)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

    # Expect 415 (invalid json)
    r = s.post("http://wulf.local/sys/", data=invalid)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

    # Expect 411 (no content)
    r = s.post("http://wulf.local/sys/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
