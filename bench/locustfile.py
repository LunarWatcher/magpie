import time
from locust import task, between, User
import httpx

class BenchUser(User):
    wait_time = between(0, 1)

    @task
    def index(self):
        
        start = time.time()
        try:
            with httpx.Client(http2=True, verify=False) as client:
                res = client.get(
                    str(self.host) + "/",
                )
            end = time.time() - start


            if res.status_code == 200:
                self.environment.events.request.fire(
                    request_type="GET",
                    name="/",
                    response_time=end,
                    response_length=len(res.content)
                )
            else:
                self.environment.events.request.fire(
                    request_type="GET",
                    name="/",
                    response_time=end,
                    exception=Exception("HTTP/{}".format(res.status_code)),
                    response_length=len(res.content)
                )
        except Exception as e:
                self.environment.events.request.fire(
                    request_type="GET",
                    name="/",
                    response_time=time.time() - start,
                    response_length=0,
                    exception=e
                )
