mod = 2147364748


def forge(val, num):
    return (val * num) % mod


class Target:
    def __init__(self, target_ip, flag, threshold):
        self.target_ip = target_ip
        self.flag = flag
        self.threshold = threshold

    def hash_ftn(self, bot, local, auth):
        return (forge(bot.nonce, 100000007) + forge(bot.bot_ip, 1000000009) + forge(local.local_ip, 23) + forge(auth.auth_ip, 101)) % mod

    def validate_puzzle(self, bot):
        print("Target server got packet from bot", bot.bot_ip)
        return bot.hash_ftn() < self.threshold
