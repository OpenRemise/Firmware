import json, requests

run_delete_requests = False
run_get_requests = False
run_put_requests = False

valid = '{"address":100,"name":"MoFo","functions":1234,"speed":42,"dir":1}'
invalid = '{"address":: '

# Open session
s = requests.Session()

""" 
DELETE
"""
if run_delete_requests:
    r = s.delete("http://remise.local/dcc/locos/3")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

"""
GET
"""
if run_get_requests:
    r = s.get("http://remise.local/dcc/locos/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

    r = s.get("http://remise.local/dcc/locos/3")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

"""
PUT
"""
if run_put_requests:
    r = s.put("http://remise.local/dcc/locos/3", data=valid)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

    # Expect 415 (invalid json)
    r = s.put("http://remise.local/dcc/locos/3", data=invalid)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))

    # Expect 411 (no content)
    r = s.put("http://remise.local/dcc/locos/3")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
