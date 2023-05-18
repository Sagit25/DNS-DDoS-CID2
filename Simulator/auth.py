import random

class Auth():
    def __init__(self, auth_ip, target_ip):
        self.target_ip = target_ip
        self.auth_ip = auth_ip
    
        self.token = 0
        
    def send_seed(self):
        return random.randint(1, 100)
    
    def hash_ftn(self, bot, local):
        bot_ip, local_ip, auth_ip, time, seed, nonce