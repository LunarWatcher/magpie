import time
from locust import HttpUser, task, between

class BenchUser(HttpUser):
    wait_time = between(0, 1)

    @task
    def index(self):
        self.client.get("/")

    def on_start(self):
        self.client.verify = False;
