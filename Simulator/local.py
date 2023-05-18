class Local:
    def __init__(self, local_ip, auth, chain_len):
        self.local_ip = local_ip
        self.auth = auth
        self.chain_len = chain_len

        self.hash_chain = []
        self.seed = None

    def hash_ftn(self, bot):
        return self.auth.hash_ftn(bot, self)

    def gen_hash_chain(self):
        token = self.seed
        for i in range(self.chain_len):
            token = self.hash_ftn(token)
            self.hash_chain.append(token)
            self.hash_chain.reverse()

    def send_puzzle_token(self):
        if len(self.hash_chain) <= 0:
            self.get_seed()
        token = self.hash_chain[0]
        self.hash_chain = self.hash_chain[1:]
        return token

    def get_seed(self):
        self.seed = self.auth.send_seed()
        self.gen_hash_chain()


