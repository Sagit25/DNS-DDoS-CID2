
// add it to net/tcp.h
#define TCPOPT_PUZZLEFLAG 10
#define TCPOPT_PUZZLETYPE 11
#define TCPOPT_PUZZLE 12
#define TCPOPT_NONCE 13

// Length of Value +2 (Byte)
#define TCPOLEN_PUZZLEFLAG 2
#define TCPOLEN_PUZZLETYPE 3 
#define TCPOLEN_PUZZLE 6
#define TCPOLEN_NONCE 6


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/init.h>
#include <linux/tcp.h>
#include <linux/list.h>
#include <net/ip.h>
#include <net/tcp.h>

// Network Hooking
static struct nf_hook_ops nf_hook_in, nf_hook_out;
const char DNS_IP[] = "0.0.0.0"; // can we store it on some config file?

struct puzzle_record {
	__u32 ip;
	__u32 puzzle;
	struct list_head head;

};


// hook before tcp_rcv. parse first and check puzzle flag
unsigned int hook_input(
		void * priv, 
		struct sk_buff *skb,
		const struct nf_hook_state * state)
{

	struct net * net = dev_net(skb->dev);
	struct tcphdr * tcp_header;
	bool refcounted;

	printk(KERN_ALERT "hooked in\n");

	// parsing tcp header and socket
	printk(KERN_ALERT "protocol : %d\n", ip_hdr(skb)->protocol);
	if(ip_hdr(skb)->protocol != 6) // 6 : TCP, 17 : UDP
		goto pass_it;

	if(skb->pkt_type != PACKET_HOST)
		goto pass_it;

	if(likely(skb_transport_header_was_set(skb))) {
		tcp_header = tcp_hdr(skb);


		// add it to tcp_input.c tcp_parse_options()
		int length = (tcp_header->doff * 4) - sizeof(struct tcphdr);

		const unsigned char *ptr;
		ptr = (const unsigned char *)(tcp_header + 1);

		bool puzzle_flag = false;
		__u8 puzzle_type = 0;
		u32 puzzle = 0, nonce = 0;
		while(length > 0) {
			int opcode = *ptr++;
			int opsize;

			switch(opcode) {
			case TCPOPT_EOL:
				goto parse_opt_done;
			case TCPOPT_NOP:
				length--;
				continue;
			default:
				if(length < 2)
					goto parse_opt_done;
				opsize = *ptr++;
				if(opsize < 2)
					goto parse_opt_done;
				if(opsize > length)
					goto parse_opt_done;
				switch(opcode) { // implemented only what we needs
					case TCPOPT_PUZZLEFLAG:
						if (opsize == TCPOLEN_PUZZLEFLAG)
							puzzle_flag = true;
						continue;
					case TCPOPT_PUZZLETYPE:
						if (opsize == TCPOLEN_PUZZLETYPE)
							puzzle_type = *(__u8 *)ptr;
						continue;
					case TCPOPT_PUZZLE:
						if (opsize == TCPOLEN_PUZZLE)
							puzzle = get_unaligned_be32(ptr);
						continue;
					case TCPOPT_NONCE:
						if (opsize == TCPOLEN_NONCE)
							puzzle = get_unaligned_be32(ptr);
						continue;
					default:
						continue;
				}

				ptr += opsize -2;
				length -= opsize;

			}
		}
parse_opt_done:

		if(puzzle_flag) {
			printk(KERN_ALERT "puzzle flag raised\n");
			//record ip with puzzleflag
			if(puzzle == 0) {
				printk(KERN_ALERT "but no puzzle data\n");
				// send request to dns server
			} else {
				printk(KERN_ALERT "puzzle : %d\n",puzzle);
				//record puzzle for ip
			}
		} else {
			//unrecord ip
		}

//		sk = __inet_lookup_skb(net->ipv4.tcp_death_row.hashinfo, skb, __tcp_hdrlen(tcp_header), tcp_header->source, tcp_header->dest, sdif, &refcounted);


//		if(!sk)
//			goto pass_it;

/*		if(1){//tcp header has puzzle flag) {

			// add puzzle flag record
			if(sk->sk_state == TCP_SYN_SENT) { // was trying connect
				if(1) { // we have puzzle
					;
				} else {
					// get puzzle from dns
				}
				// solve puzzle function
				// resend connect query

				return NF_DROP; // drop current packet and wait server request resended connect packet
			}
		} else {
			// remove puzzle flag record
		}*/
	}

pass_it:
	return NF_ACCEPT;
}

unsigned int hook_output (
		void * priv,
		struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	printk(KERN_ALERT "hooked out\n");

	// if (dest_ip in record) {
	//   if puzzle is stored in record {
	//        pop(puzzle)
	//   } else {
	//      ask puzzle to dns
	//   }
	//  solve puzzle();
	//  add data to skb;

	return NF_ACCEPT;
}

int __init hook_init(void) {

	nf_hook_in.hook = hook_input;
	nf_hook_in.hooknum = NF_INET_PRE_ROUTING;
	nf_hook_in.pf = PF_INET;
	nf_hook_in.priority = NF_IP_PRI_FIRST;

	nf_hook_out.hook = hook_output;
	nf_hook_out.hooknum = NF_INET_POST_ROUTING;
	nf_hook_out.pf = PF_INET;
	nf_hook_out.priority = NF_IP_PRI_FIRST;

	nf_register_net_hook(&init_net, &nf_hook_in);
	nf_register_net_hook(&init_net, &nf_hook_out);

	return 0;
}
void __exit hook_exit(void) {

	nf_unregister_net_hook(&init_net, &nf_hook_in);
	nf_unregister_net_hook(&init_net, &nf_hook_out);
}

module_init(hook_init);
module_exit(hook_exit);

MODULE_AUTHOR("MINJUN SUN. oenothera@snu.ac.kr");
MODULE_DESCRIPTION("Hook Network Function to insert DDoS Mitigation Logic");
MODULE_LICENSE("GPL");
