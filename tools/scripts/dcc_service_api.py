import json, requests

run_post_request = False
run_get_requests = False
run_put_requests = False

valid = '{"1":null,"2":1,"3":12,"5":120,"8":null,"18":null,"19":null}'
valid_read_cv1 = '{"1":null}'

# Open session
s = requests.Session()

"""
POST (to enter service mode)
"""
if run_post_request:
    r = s.post("http://wulf.local/dcc/", data={"mode": "service"})
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

"""
GET
"""
if run_get_requests:
    r = s.get("http://wulf.local/dcc/service/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

"""
PUT
"""
if run_put_requests:
    r = s.put("http://wulf.local/dcc/service/", data=valid)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

    # Expect 411 (no content)
    r = s.put("http://wulf.local/dcc/service/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
