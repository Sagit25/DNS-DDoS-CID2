class Bot():
    def __init__(self, bot_ip, local):
        self.bot_ip = bot_ip
        self.local = local
        
        self.nonce = 0
        self.time = 0
        self.threshold = 0
        self.token = 0
    
    def get_puzzle_token(self, local):
        self.token = local.send_puzzle_token()
        
    def hash_ftn(self):
        self.local.hash_ftn(self)
    
    def solve_puzzle(self):