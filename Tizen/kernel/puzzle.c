#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/linkage.h>
#include <linux/types.h>
#include <crypto/hash.h>
#define PLAIN_LENGTH 13
#define SHA256_LENGTH 32


__u32 do_puzzle_hash(__u32 nonce, __u32 target_ip, __u32 target_port, __u8 puzzle_type) {
    unsigned char plaintext[PLAIN_LENGTH];
    unsigned char hash_sha256[SHA256_LENGTH];
    // char *plaintext = "This is a test";
    __u32 i, j = 0;
    for(i = 0; i < 4; ++i, ++j) {
        plaintext[j] = nonce & 255;
        nonce >>= 8;
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

__u32 do_puzzle_solve(__u32 threshold, __u32 target_ip, __u32 target_port, __u8 puzzle_type) {
    __u32 nonce;
    for(nonce = 1; nonce > 0; ++nonce) {
        if(get_hash(nonce, target_ip, target_port, puzzle_type) < threshold) {
            return nonce;
        }
    }
    return 0;
}

SYSCALL_DEFINE4(puzzle_hash, __u32, nonce, __u32, target_ip, __u32, target_port, __u8, puzzle_type) {
    return do_puzzle_hash(nonce, target_ip, target_port, puzzle_type);
}


SYSCALL_DEFINE4(puzzle_solve, __u32, threshold, __u32, target_ip, __u32, target_port, __u8, puzzle_type) {
    return do_puzzle_solve(threshold, target_ip, target_port, puzzle_type);
}