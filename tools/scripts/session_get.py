import datetime, requests

# https://requests.readthedocs.io/en/latest/user/advanced/
s = requests.Session()
finish_time = datetime.datetime.now() + datetime.timedelta(hours=2)
while datetime.datetime.now() < finish_time:
    s.get("http://remise.local/main.dart.js")
