import random


class Auth:
    def __init__(self, auth_ip, target):
        self.target = target
        self.auth_ip = auth_ip

        self.token = 0

    def send_seed(self):
        return random.randint(1, 100)

    def hash_ftn(self, bot, local):
        return self.target.hash_ftn(bot, local, self)

