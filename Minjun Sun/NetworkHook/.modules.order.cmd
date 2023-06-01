cmd_/home/ubuntu/NetworkHook/modules.order := {   echo /home/ubuntu/NetworkHook/NetworkHook.ko; :; } | awk '!x[$$0]++' - > /home/ubuntu/NetworkHook/modules.order
