from threading import Thread
from multiprocessing import Process, Queue

from bot import Bot
from auth import Auth
from target import Target
from local import Local

if __name__ == "__main__":
    local_ip = 1015
    auth_ip = 1025
    target_ip = 1035
    chain_len = 100
    bot_num = 20
    threshold = 1000
    flag = False

    target = Target(target_ip, flag, threshold)
    auth = Auth(auth_ip, target)
    local = Local(local_ip, auth, chain_len)
    bot_list = [Bot(x, local, threshold) for x in range(bot_num)]


    for bot in bot_list:
        bot.solve_puzzle()
        target.validate_puzzle(bot)
