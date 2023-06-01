class Bot:
    def __init__(self, bot_ip, local, threshold):
        self.bot_ip = bot_ip
        self.local = local

        self.nonce = 0
        self.time = 0
        self.threshold = threshold
        self.token = 0

    def get_puzzle_token(self, local):
        self.token = local.send_puzzle_token()

    def hash_ftn(self):
        return self.local.hash_ftn(self)

    def solve_puzzle(self):
        self.nonce = 0
        for i in range(2147483648):
            if self.hash_ftn() < self.threshold:
                print("Bot", self.bot_ip, "solved the puzzle with nonce", self.nonce)
                break
            self.nonce += 1
