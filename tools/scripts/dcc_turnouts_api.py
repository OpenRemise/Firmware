import json, requests

run_get_requests = True
run_put_requests = False
run_delete_requests = False

valid = '{"address":100,"name":"T72"}'
invalid = '{"address":: '

# Open session
s = requests.Session()

"""
GET
"""
if run_get_requests:
    r = s.get("http://remise.local/dcc/turnouts/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

    r = s.get("http://remise.local/dcc/turnouts/3")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())
