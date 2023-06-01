#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <net/puzzle.h>

#include <linux/linkage.h>
#include <linux/types.h>
#include <crypto/hash.h>


// added by junjinyong
#define PLAIN_LENGTH 17
#define SHA256_LENGTH 32

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

__u32 do_puzzle_solve(__u32 threshold, __u32 puzzle, __u32 target_ip, __u32 target_port, __u8 puzzle_type) {
    __u32 nonce;
    for(nonce = 1; nonce > 0; ++nonce) {
        if(do_puzzle_hash(nonce, puzzle, target_ip, target_port, puzzle_type) < threshold) {
            return nonce;
        }
    }
    return 0;
}

SYSCALL_DEFINE5(puzzle_hash, __u32, nonce, __u32, puzzle, __u32, target_ip, __u32, target_port, __u8, puzzle_type) {
    return do_puzzle_hash(nonce, puzzle, target_ip, target_port, puzzle_type);
}


SYSCALL_DEFINE5(puzzle_solve, __u32, threshold, __u32, puzzle, __u32, target_ip, __u32, target_port, __u8, puzzle_type) {
    return do_puzzle_solve(threshold, puzzle, target_ip, target_port, puzzle_type);
}


//



static inline u8 __down_to_u8(u16 val) {
    return (u8)(val < MAX_SPARE_GAP ? val : MAX_SPARE_GAP);
}

static const u32 NOT_FOUND = 500;

LIST_HEAD(policy_head);
LIST_HEAD(cache_head);
u32 dns_ip = 0;
EXPORT_SYMBOL(dns_ip);

u32 solve_puzzle(u8 type, u32 puzzle, u32 ip, u32 sub_ip) {

    u32 nonce = 0;

    switch(type) {
        case PZLTYPE_NONE:
            goto no_puzzle;

        case PZLTYPE_COPY:
        case PZLTYPE_INC:
        case PZLTYPE_DNS_COPY:
        case PZLTYPE_DNS_INC:
            return do_puzzle_solve(4096, puzzle, ip, sub_ip, type);
    }    
no_puzzle:
    return 0;
}
EXPORT_SYMBOL(solve_puzzle);



bool check_nonce(u8 type, u32 puzzle, u32 nonce, u32 ip, u32 sub_ip) {
    printk(KERN_INFO "type : %u, puzzle : %u, nonce : %u", type, puzzle, nonce);
    return do_puzzle_hash(nonce, puzzle, ip, sub_ip, type);
}

u32 __generate_seed(u8 type, u32 ip) {
    /* TODO */
    return 10;
}

static bool find_puzzle_policy(u32 ip, struct puzzle_policy** ptr) {
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

static bool find_puzzle_cache(u32 ip, struct puzzle_cache** ptr) {
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

static void __print_policy_detail(struct puzzle_policy* policy) {

    printk(KERN_INFO "ip : %u.%u.%u.%u type : %d\n"
                , (policy->ip       )%256
                , (policy->ip  >>  8)%256
                , (policy->ip  >> 16)%256
                , (policy->ip  >> 24), policy->puzzle_type);
    printk(KERN_INFO "    | seed : %u , old_seed : %u\n", policy->seed, policy->seed_old);
    printk(KERN_INFO "    | assigned length : %u\n,", policy->assigned_length);
    printk(KERN_INFO "    | available : %u + %u\n,", policy->latest_pos, policy->spare_gap);
}

int print_policy_detail(u32 ip) {
    struct puzzle_policy* policy;
    printk(KERN_INFO "--puzzle_policy_detail--\n");
    if(find_puzzle_policy(ip, &policy))
        __print_policy_detail(policy);
    else
        return 0;
    printk(KERN_INFO "------------------------\n");
    return 1;
}

SYSCALL_DEFINE1(puzzle_detail_policy, u32, ip)
{
    return print_policy_detail(ip);
}


int print_policy(void) {
    struct puzzle_policy* policy;
    struct list_head* ptr;
    int count = 0;
    printk(KERN_WARNING "--puzzle_policy_all-----\n");
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

int print_cache(void) {
    struct puzzle_cache* cache;
    struct list_head* ptr;
    int count = 0;
    printk(KERN_INFO "--puzzle_cache-----\n");
    list_for_each(ptr, &cache_head) {
        cache = list_entry(ptr, struct puzzle_cache, list);
        printk(KERN_INFO "ip : %u.%u.%u.%u type : %d\n"
                    , (cache->ip       )%256
                    , (cache->ip  >>  8)%256
                    , (cache->ip  >> 16)%256
                    , (cache->ip  >> 24), cache->puzzle_type);
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


int add_policy(u32 ip, u8 puzzle_type, u16 assigned_length) {
    struct puzzle_policy* policy;
    if(find_puzzle_policy(ip, &policy))
        return -1;
    
    policy = kmalloc(sizeof(*policy), GFP_KERNEL);
    memset(policy, 0, sizeof(*policy));

    policy->ip = ip;
    policy->puzzle_type = puzzle_type;
    policy-> assigned_length = 2;

    list_add_tail(&(policy->list), &policy_head);


    return 0;
}
EXPORT_SYMBOL(add_policy);
SYSCALL_DEFINE3(puzzle_add_policy, u32, ip, u8, puzzle_type, u16, assigned_length)
{
    return add_policy(ip, puzzle_type, assigned_length);
}

static void update_to_new_seed(struct puzzle_policy* policy, u32 new_seed) {

    policy->seed_old = policy->seed;
    policy->seed = new_seed;
    policy->spare_gap = __down_to_u8(policy->latest_pos);
    policy->latest_pos = policy->assigned_length;
}

int generate_new_seed(u32 ip) {
    struct puzzle_policy* policy;
    if(find_puzzle_policy(ip, &policy))
        return -1;

    update_to_new_seed(policy, __generate_seed(policy->puzzle_type, policy->ip));
    return 0;

}
EXPORT_SYMBOL(generate_new_seed);
SYSCALL_DEFINE1(puzzle_remake_seed, u32, ip)
{
    return generate_new_seed(ip);
}



bool find_pos_of_puzzle(u32 ip, u32 puzzle, u16* pos) {
    struct puzzle_policy* policy;
    u32 hash_value, iter, acceptable_pos;
    iter = 0;
    *pos = NOT_FOUND;
    if(unlikely(!find_puzzle_policy(ip, &policy))) 
        return false;
    hash_value = policy->seed;
    acceptable_pos = policy->latest_pos + policy->spare_gap;
    while( iter < policy->assigned_length ) {
        if(hash_value == puzzle) {
            if(*pos == NOT_FOUND || iter < acceptable_pos)
                *pos = iter;
        }
        iter ++;
        hash_value = do_puzzle_hash(hash_value, 0, 0, 0, 0);
    }
    hash_value = policy->seed;
    while( iter < acceptable_pos ) {
        if(hash_value == puzzle) {
            *pos = iter;
        }
        iter++;
        hash_value = do_puzzle_hash(hash_value, 0, 0, 0, 0);
    }
    return true;
}
EXPORT_SYMBOL(find_pos_of_puzzle);

int update_policy_from_config(void) {
    //TODO
    return 0;
}
SYSCALL_DEFINE0(puzzle_update_policy)
{
    return update_policy_from_config();
}

int update_policy(u32 ip, u8 type, u32 seed, u16 length) {
    struct puzzle_policy* policy;
    if(unlikely(!find_puzzle_policy(ip, &policy)))
        return -1;

    if(type) {
        if(type == PZLTYPE_NONE) {
            list_del(&(policy->list));
            kfree(policy);

            return 1;
        }
        policy->puzzle_type = type;
    }

    if(seed) {
        policy->seed = seed;
        update_to_new_seed(policy, seed);
    }

    return 0;
}

SYSCALL_DEFINE4(puzzle_edit_policy, u32, ip, u8, puzzle_type, u32, seed, u16, assigned_length)
{
    return update_policy(ip, puzzle_type, seed, assigned_length);
}

int update_policy_type(u32 ip, u8 type) {
    return update_policy(ip, type, 0, 0);
}
EXPORT_SYMBOL(update_policy_type);

int update_policy_length(u32 ip, u16 length) {
   return update_policy(ip, 0, 0, length);
}
EXPORT_SYMBOL(update_policy_length);

int update_puzzle_cache(u32 ip, u32 puzzle_type, u32 puzzle) {
    struct puzzle_policy* policy;
    if(unlikely(!find_puzzle_policy(ip, &policy)))
        return -1;

    return 0;
    /*TODO*/
}
EXPORT_SYMBOL(update_puzzle_cache);