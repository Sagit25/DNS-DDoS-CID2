class Target():
    def __init__(self, target_ip, flag):
        self.target_ip = target_ip
        
        self.flag = flag
        
    def hash_ftn(self, bot, local, auth):
        pass
        
    def validate_puzzle(self, bot):
        return bot.hash_ftn() > self.threshold:
            