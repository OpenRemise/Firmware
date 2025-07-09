import json, requests

run_get_requests = False
run_post_requests = False

valid = '{"main_current":0,"prog_current":0,"filtered_main_current":0,"temperature":0,"supply_voltage":0,"vcc_voltage":0,"central_state":2,"central_state_ex":0,"capabilities":127}'
invalid = '{"main_current":: '

# Open session
s = requests.Session()

"""
GET
"""
if run_get_requests:
    r = s.get("http://remise.local/dcc/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

"""
POST
"""
if run_post_requests:
    r = s.post("http://remise.local/dcc/", data=valid)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
