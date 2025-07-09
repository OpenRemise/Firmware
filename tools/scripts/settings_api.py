import json, requests


run_get_requests = True
run_post_requests = True

valid = '{"sta_mdns":"some DNS","sta_ssid":"some SSID","sta_pass":"some PASS","dcc_preamble":42,"dcc_bit1_dur":43,"dcc_bit0_dur":44,"dcc_bidibit_dur":60}'
valid_no_sta = (
    '{"dcc_preamble":42,"dcc_bit1_dur":43,"dcc_bit0_dur":44,"dcc_bidibit_dur":60}'
)

# Open session
s = requests.Session()

"""
GET
"""
if run_get_requests:
    r = s.get("http://remise.local/settings/")
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
    if len(r.content) and r.headers.get("content-type") == "application/json":
        print(r.json())

"""
POST
"""
if run_post_requests:
    r = s.post("http://remise.local/settings/", data=valid_no_sta)
    print(str(r.request) + " " + str(r.url) + " " + str(r.status_code))
