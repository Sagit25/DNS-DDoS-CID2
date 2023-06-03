#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/types.h>
#include <net/puzzle.h>

#include <crypto/hash.h>

// added by junjinyong
#define PLAIN_LENGTH 17
#define SHA256_LENGTH 32

static const u32 NOT_FOUND = 500;

LIST_HEAD(policy_head);
LIST_HEAD(cache_head);
static struct puzzel_dns_info {
    u32 ip;
    u32 port;
} puzzle_dns = {0, 0};
static u8 puzzle_type = PZLTYPE_NONE;
static u32 last_seed = 1;

__u32 do_puzzle_hash(__u32 nonce, __u32 puzzle, __u32 target_ip, __u32 target_port, __u8 puzzle_type) {
    unsigned char plaintext[PLAIN_LENGTH];
    unsigned char hash_sha256[SHA256_LENGTH];
    // char *plaintext = "This is a test";
    __u32 i, j = 0;
    for(i = 0; i < 4; ++i, ++j) {
        plaintext[j] = nonce & 255;
        nonce >>= 8;
    }
    for(i = 0; i < 4; ++i, ++j) {
        plaintext[j] = puzzle & 255;
        puzzle >>= 8;
    }
    for(i = 0; i < 4; ++i, ++j) {
        plaintext[j] = target_ip & 255;
        target_ip >>= 8;
    }
    for(i = 0; i < 4; ++i, ++j) {
        plaintext[j] = target_port & 255;
        target_port >>= 8;
    }
    for(i = 0; i < 1; ++i, ++j) {
        plaintext[j] = puzzle_type & 255;
        puzzle_type >>= 8;
    }
    struct crypto_shash *sha256 = crypto_alloc_shash("sha256", 0, 0);
    __u32 size = sizeof(struct shash_desc) + crypto_shash_descsize(sha256);
    //sha256 = crypto_alloc_shash("md5", 0, CRYPTO_ALG_ASYNC);
    struct shash_desc *shash = kmalloc(size, GFP_KERNEL);
    
    
    if(sha256 == NULL) {
        return 0;
    }
    shash -> tfm = sha256;
    
    crypto_shash_init(shash);
    crypto_shash_update(shash, plaintext, PLAIN_LENGTH);
    // crypto_shash_update(shash, plaintext, strlen(plaintext));
    crypto_shash_final(shash, hash_sha256);
    crypto_free_shash(sha256);
    kfree(shash);
    __u32 result = 0;
    __u32 offset, temp;
    for(i = 0; i < 4; ++i) {
        result = result << 8;
        temp = 0;
        offset = i << 3;
        for(j = 0; j < 8; ++j) {
            temp = temp ^ hash_sha256[offset + j];
        }
        result = result + temp;
    }
    return result;
}
SYSCALL_DEFINE5(puzzle_hash, __u32, nonce, __u32, puzzle, __u32, target_ip, __u32, target_port, __u8, puzzle_type) {
    return do_puzzle_hash(nonce, puzzle, target_ip, target_port, puzzle_type);
}

u32 do_puzzle_solve(__u32 threshold, __u32 puzzle, __u32 target_ip, __u32 target_port, __u8 puzzle_type) {
    __u32 nonce;
    for(nonce = 1; nonce > 0; ++nonce) {
        if(do_puzzle_hash(nonce, puzzle, target_ip, target_port, puzzle_type) < threshold) {
            return nonce;
        }
    }
    return 0;
}
EXPORT_SYMBOL(do_puzzle_solve);
SYSCALL_DEFINE5(puzzle_solve, __u32, threshold, __u32, puzzle, __u32, target_ip, __u32, target_port, __u8, puzzle_type) {
    return do_puzzle_solve(threshold, puzzle, target_ip, target_port, puzzle_type);
}

bool find_puzzle_policy(u32 ip, struct puzzle_policy** ptr) {
    struct puzzle_policy* policy;
    struct list_head* head;
    list_for_each(head, &policy_head) {
        policy = list_entry(head, struct puzzle_policy, list);
        if(ip == policy->ip) {
            *ptr = policy;
            return true;
        }
    }

    return false;
}
EXPORT_SYMBOL(find_puzzle_policy);

bool find_puzzle_cache(u32 ip, struct puzzle_cache** ptr) {
    struct puzzle_cache* cache;
    struct list_head* head;
    list_for_each(head, &cache_head) {
        cache = list_entry(head, struct puzzle_cache, list);
        if(ip == cache->ip) {
            *ptr = cache;
            return true;
        }
    }

    return false;
}
EXPORT_SYMBOL(find_puzzle_cache);

static void __print_policy_detail(struct puzzle_policy* policy) {
    u32 ip = htonl(policy->ip);

    printk(KERN_INFO "ip : %u.%u.%u.%u "
                , (ip       )%256
                , (ip  >>  8)%256
                , (ip  >> 16)%256
                , (ip  >> 24));
    printk(KERN_INFO "    | seed : %u , old_seed : %u\n", policy->seed, policy->seed_old);
    printk(KERN_INFO "    | assigned length : %u\n,", policy->assigned_length);
    printk(KERN_INFO "    | available : %u + %u\n,", policy->latest_pos, policy->spare_gap);
}

long print_policy_detail(u32 ip) {
    struct puzzle_policy* policy;
    printk(KERN_INFO "--puzzle_policy_detail---");
    switch(puzzle_type) {
    case PZLTYPE_NONE: 
        printk(KERN_INFO "NONE--\n");
        break;
    case PZLTYPE_LOCAL: 
        printk(KERN_INFO "LOCAL-\n");
        break;
    case PZLTYPE_DNS: 
        printk(KERN_INFO "DNS---\n");
        break;
    }
    if(find_puzzle_policy(ip, &policy))
        __print_policy_detail(policy);
    else
        return 0;
    printk(KERN_INFO "------------------------\n");
    return 1;
}

SYSCALL_DEFINE1(puzzle_detail_policy, __u32, ip)
{
    return print_policy_detail(ip);
}

long print_policy(void) {
    struct puzzle_policy* policy;
    struct list_head* ptr;
    int count = 0;
    printk(KERN_WARNING "--puzzle_policy_all---");
    switch(puzzle_type) {
    case PZLTYPE_NONE: 
        printk(KERN_INFO "NONE--\n");
        break;
    case PZLTYPE_LOCAL: 
        printk(KERN_INFO "LOCAL-\n");
        break;
    case PZLTYPE_DNS: 
        printk(KERN_INFO "DNS---\n");
        break;
    }
    list_for_each(ptr, &policy_head) {
        policy = list_entry(ptr, struct puzzle_policy, list);
        __print_policy_detail(policy);
        count ++;
    }
    printk(KERN_WARNING "---------------count : %d\n", count);

    return count;
}
EXPORT_SYMBOL(print_policy);
SYSCALL_DEFINE0(puzzle_print_policy)
{
    return print_policy();
}

long print_cache(void) {
    struct puzzle_cache* cache;
    struct list_head* ptr;
    u32 ip;
    int count = 0;
    printk(KERN_INFO "--puzzle_cache-----\n");
    list_for_each(ptr, &cache_head) {
        cache = list_entry(ptr, struct puzzle_cache, list);
        ip = htonl(ip);
        printk(KERN_INFO "ip : %u.%u.%u.%u type : %d\n"
                    , (ip       )%256
                    , (ip  >>  8)%256
                    , (ip  >> 16)%256
                    , (ip  >> 24), cache->puzzle_type);
        printk(KERN_INFO "    | stored_puzzle : %u\n", cache->puzzle);
        count ++;
    }
    printk(KERN_INFO "---------------count : %d\n", count);

    return count;
}

SYSCALL_DEFINE0(puzzle_print_cache)
{
    return print_cache();
}


long add_policy(u32 ip, u16 assigned_length, u32 threshold) {
    struct puzzle_policy* policy;
    if(find_puzzle_policy(ip, &policy))
        return -1;
    
    policy = kmalloc(sizeof(*policy), GFP_KERNEL);
    memset(policy, 0, sizeof(*policy));

    policy->ip = ip;
    policy->assigned_length = 2;
    policy->threshold = threshold;

    list_add_tail(&(policy->list), &policy_head);

    return 0;
}
EXPORT_SYMBOL(add_policy);
SYSCALL_DEFINE3(puzzle_add_policy, __u32, ip, __u16, assigned_length, __u32, threshold)
{
    return add_policy(ip, assigned_length, threshold);
}

static u32 update_to_new_seed(struct puzzle_policy* policy, u32 new_seed) {

    policy->seed_old = policy->seed;
    policy->seed = new_seed;
    policy->spare_gap = policy->latest_pos;
    policy->latest_pos = policy->assigned_length;
    return new_seed;
}

u32 generate_new_seed(u32 ip) {
    struct puzzle_policy* policy;
    if(find_puzzle_policy(ip, &policy))
        return 0;
    last_seed = do_puzzle_hash(0, last_seed, ip, 0, puzzle_type);
    return update_to_new_seed(policy, last_seed);

}
EXPORT_SYMBOL(generate_new_seed);
SYSCALL_DEFINE1(puzzle_remake_seed, __u32, ip)
{
    return generate_new_seed(ip);
}

static u32 find_pos_of_puzzle(struct puzzle_policy* policy, u32 puzzle) {
    u32 hash_value, iter, acceptable_pos;
    iter = 0;
    u32 pos = NOT_FOUND;

    hash_value = policy->seed;
    acceptable_pos = policy->latest_pos + policy->spare_gap;
    while( iter < policy->assigned_length ) {
        if(hash_value == puzzle) {
            if(pos == NOT_FOUND || iter < acceptable_pos)
                pos = iter;
        }
        iter ++;
        hash_value = do_puzzle_hash(hash_value, 0, 0, 0, 0);
    }
    hash_value = policy->seed;
    while( iter < acceptable_pos ) {
        if(hash_value == puzzle) {
            pos = iter;
        }
        iter++;
        hash_value = do_puzzle_hash(hash_value, 0, 0, 0, 0);
    }
    return pos;
}

int update_policy_from_config(void) {
    //TODO
    return 0;
}
SYSCALL_DEFINE0(puzzle_update_policy)
{
    return update_policy_from_config();
}

long update_policy(u32 ip, u32 seed, u16 length, u32 threshold) {
    struct puzzle_policy* policy;
    if(unlikely(!find_puzzle_policy(ip, &policy)))
        return -1;
/*
    if(type) {
        if(type == PZLTYPE_NONE) {
            list_del(&(policy->list));
            kfree(policy);

            return 1;
        }
        policy->puzzle_type = type;
    }
*/
    if(seed) {
        policy->seed = seed;
        update_to_new_seed(policy, seed);
    }

    if(threshold) {
        policy->threshold = threshold;
    }

    return 0;
}
EXPORT_SYMBOL(update_policy);
SYSCALL_DEFINE4(puzzle_edit_policy, __u32, ip, __u32, seed, __u16, assigned_length, __u32, threshold)
{
    return update_policy(ip, seed, assigned_length, threshold);
}
int check_puzzle(u8 type, u32 puzzle, u32 nonce, u32 ip, u32 port, u32 policy_ip) {
    printk(KERN_INFO "type : %u, puzzle : %u, nonce : %u", type, puzzle, nonce);
    struct puzzle_policy* policy;
    u32 pos;

    if(unlikely(!find_puzzle_policy(policy_ip, &policy)))
        return type == PZLTYPE_DNS ? 1 : 0;
    
    if(unlikely(puzzle_type != type))
        return 1;

    pos = find_pos_of_puzzle(policy, puzzle);
    if(pos == NOT_FOUND || pos == (u32)policy->latest_pos 
        || pos >= (u32)policy->latest_pos + policy->spare_gap)
        return 1;

    if(do_puzzle_hash(nonce, puzzle, ip, policy_ip, type) >= policy->threshold)
        return 1;

    if(pos < (u32)(policy->latest_pos)) {
        policy->spare_gap = policy->latest_pos - (u16) pos;
        policy->latest_pos = (u16) pos;
    } else {
        policy->spare_gap = (u16) (pos - policy->latest_pos);
    }

    return 0;
}
EXPORT_SYMBOL(check_puzzle);

int update_puzzle_cache(u32 ip, u8 type, u32 puzzle, u32 threshold) {
    struct puzzle_cache* cache;
    int updated = 0;
    if(unlikely(!find_puzzle_cache(ip, &cache))) {
        if(type == PZLTYPE_NONE)
            return 0;

        cache = kmalloc(sizeof(*cache), GFP_KERNEL);
        memset(cache, 0, sizeof(*cache));

        cache->ip = ip;
        cache->puzzle_type = type;
        cache->puzzle = puzzle;
        cache->threshold = threshold;

        list_add_tail(&(cache->list), &cache_head);
        return 4;
    }

    if(type) {
        if(type == PZLTYPE_NONE) {
            list_del(&(cache->list));
            kfree(cache);

            return 4;
        }
        cache->puzzle_type = type;
    }

    if(puzzle && cache->puzzle != puzzle) {
        updated ++;
        cache->puzzle = puzzle;
    }
    if(threshold && cache->threshold != threshold) {
        updated ++;
        cache->threshold = threshold;
    }

    return updated;
}
EXPORT_SYMBOL(update_puzzle_cache);

u8 get_puzzle_type() {
    return puzzle_type;
}
EXPORT_SYMBOL(get_puzzle_type);
SYSCALL_DEFINE0(puzzle_get_type) {
    return (long)get_puzzle_type();
}

u8 set_puzzle_type(u8 type) {
    switch(type) {
    case PZLTYPE_NONE:
    case PZLTYPE_LOCAL:
    case PZLTYPE_DNS:
        puzzle_type = type;
        break;
    default:
        break;
    }

    return puzzle_type;
}
EXPORT_SYMBOL(set_puzzle_type);
SYSCALL_DEFINE1(puzzle_set_type, __u8, type) {
    return (long)set_puzzle_type(type);
}

long get_puzzle_dns(u32* ip, u32* port) {
    *ip = puzzle_dns.ip;
    *port = puzzle_dns.port;

    return 0;
}
EXPORT_SYMBOL(get_puzzle_dns);

long print_puzzle_dns(void) {
    printk("ip : %I4, port: %u\n", puzzle_dns.ip, puzzle_dns.port);

    return 0;
}
SYSCALL_DEFINE0(puzzle_print_dns) {
    return print_puzzle_dns();
}

long set_puzzle_dns(u32 ip, u32 port) {
    puzzle_dns.ip = ip;
    puzzle_dns.port = port;

    print_puzzle_dns();
    return 0;
}
SYSCALL_DEFINE2(puzzle_set_dns, __u32, ip, __u32, port) {
    return set_puzzle_dns(ip, port);
}