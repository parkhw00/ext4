#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "ext4.h"

#define fatal(fmt, args...)                                            \
    do                                                                 \
    {                                                                  \
        e->message_cb(e->priv, true, __func__, __LINE__, fmt, ##args); \
    } while (0)

#define debug(fmt, args...)                                             \
    do                                                                  \
    {                                                                   \
        e->message_cb(e->priv, false, __func__, __LINE__, fmt, ##args); \
    } while (0)

typedef uint64_t __le64;
typedef uint32_t __u32;
typedef uint32_t __le32;
typedef uint16_t __le16;
typedef uint8_t __u8;

// https://ext4.wiki.kernel.org/index.php/Ext4_Disk_Layout
struct super_block
{
    __le32 s_inodes_count;            // 0x0
    __le32 s_blocks_count_lo;         // 0x4
    __le32 s_r_blocks_count_lo;       // 0x8
    __le32 s_free_blocks_count_lo;    // 0xC
    __le32 s_free_inodes_count;       // 0x10
    __le32 s_first_data_block;        // 0x14
    __le32 s_log_block_size;          // 0x18
    __le32 s_log_cluster_size;        // 0x1C
    __le32 s_blocks_per_group;        // 0x20
    __le32 s_clusters_per_group;      // 0x24
    __le32 s_inodes_per_group;        // 0x28
    __le32 s_mtime;                   // 0x2C
    __le32 s_wtime;                   // 0x30
    __le16 s_mnt_count;               // 0x34
    __le16 s_max_mnt_count;           // 0x36
    __le16 s_magic;                   // 0x38
    __le16 s_state;                   // 0x3A
    __le16 s_errors;                  // 0x3C
    __le16 s_minor_rev_level;         // 0x3E
    __le32 s_lastcheck;               // 0x40
    __le32 s_checkinterval;           // 0x44
    __le32 s_creator_os;              // 0x48
    __le32 s_rev_level;               // 0x4C
    __le16 s_def_resuid;              // 0x50
    __le16 s_def_resgid;              // 0x52
    __le32 s_first_ino;               // 0x54
    __le16 s_inode_size;              // 0x58
    __le16 s_block_group_nr;          // 0x5A
    __le32 s_feature_compat;          // 0x5C
    __le32 s_feature_incompat;        // 0x60
    __le32 s_feature_ro_compat;       // 0x64
    __u8 s_uuid[16];                  // 0x68
    char s_volume_name[16];           // 0x78
    char s_last_mounted[64];          // 0x88
    __le32 s_algorithm_usage_bitmap;  // 0xC8
    __u8 s_prealloc_blocks;           // 0xCC
    __u8 s_prealloc_dir_blocks;       // 0xCD
    __le16 s_reserved_gdt_blocks;     // 0xCE
    __u8 s_journal_uuid[16];          // 0xD0
    __le32 s_journal_inum;            // 0xE0
    __le32 s_journal_dev;             // 0xE4
    __le32 s_last_orphan;             // 0xE8
    __le32 s_hash_seed[4];            // 0xEC
    __u8 s_def_hash_version;          // 0xFC
    __u8 s_jnl_backup_type;           // 0xFD
    __le16 s_desc_size;               // 0xFE
    __le32 s_default_mount_opts;      // 0x100
    __le32 s_first_meta_bg;           // 0x104
    __le32 s_mkfs_time;               // 0x108
    __le32 s_jnl_blocks[17];          // 0x10C
    __le32 s_blocks_count_hi;         // 0x150
    __le32 s_r_blocks_count_hi;       // 0x154
    __le32 s_free_blocks_count_hi;    // 0x158
    __le16 s_min_extra_isize;         // 0x15C
    __le16 s_want_extra_isize;        // 0x15E
    __le32 s_flags;                   // 0x160
    __le16 s_raid_stride;             // 0x164
    __le16 s_mmp_interval;            // 0x166
    __le64 s_mmp_block;               // 0x168
    __le32 s_raid_stripe_width;       // 0x170
    __u8 s_log_groups_per_flex;       // 0x174
    __u8 s_checksum_type;             // 0x175
    __le16 s_reserved_pad;            // 0x176
    __le64 s_kbytes_written;          // 0x178
    __le32 s_snapshot_inum;           // 0x180
    __le32 s_snapshot_id;             // 0x184
    __le64 s_snapshot_r_blocks_count; // 0x188
    __le32 s_snapshot_list;           // 0x190
    __le32 s_error_count;             // 0x194
    __le32 s_first_error_time;        // 0x198
    __le32 s_first_error_ino;         // 0x19C
    __le64 s_first_error_block;       // 0x1A0
    __u8 s_first_error_func[32];      // 0x1A8
    __le32 s_first_error_line;        // 0x1C8
    __le32 s_last_error_time;         // 0x1CC
    __le32 s_last_error_ino;          // 0x1D0
    __le32 s_last_error_line;         // 0x1D4
    __le64 s_last_error_block;        // 0x1D8
    __u8 s_last_error_func[32];       // 0x1E0
    __u8 s_mount_opts[64];            // 0x200
    __le32 s_usr_quota_inum;          // 0x240
    __le32 s_grp_quota_inum;          // 0x244
    __le32 s_overhead_blocks;         // 0x248
    __le32 s_backup_bgs[2];           // 0x24C
    __u8 s_encrypt_algos[4];          // 0x254
    __u8 s_encrypt_pw_salt[16];       // 0x258
    __le32 s_lpf_ino;                 // 0x268
    __le32 s_prj_quota_inum;          // 0x26C
    __le32 s_checksum_seed;           // 0x270
    __le32 s_reserved[98];            // 0x274
    __le32 s_checksum;                // 0x3FC
};

struct group_desc
{
    __le32 bg_block_bitmap_lo;      // 0x0
    __le32 bg_inode_bitmap_lo;      // 0x4
    __le32 bg_inode_table_lo;       // 0x8
    __le16 bg_free_blocks_count_lo; // 0xC
    __le16 bg_free_inodes_count_lo; // 0xE
    __le16 bg_used_dirs_count_lo;   // 0x10
    __le16 bg_flags;                // 0x12
    __le32 bg_exclude_bitmap_lo;    // 0x14
    __le16 bg_block_bitmap_csum_lo; // 0x18
    __le16 bg_inode_bitmap_csum_lo; // 0x1A
    __le16 bg_itable_unused_lo;     // 0x1C
    __le16 bg_checksum;             // 0x1E
    __le32 bg_block_bitmap_hi;      // 0x20
    __le32 bg_inode_bitmap_hi;      // 0x24
    __le32 bg_inode_table_hi;       // 0x28
    __le16 bg_free_blocks_count_hi; // 0x2C
    __le16 bg_free_inodes_count_hi; // 0x2E
    __le16 bg_used_dirs_count_hi;   // 0x30
    __le16 bg_itable_unused_hi;     // 0x32
    __le32 bg_exclude_bitmap_hi;    // 0x34
    __le16 bg_block_bitmap_csum_hi; // 0x38
    __le16 bg_inode_bitmap_csum_hi; // 0x3A
    __u32 bg_reserved;              // 0x3C
};

struct ext4fs
{
    void *priv;

    ext4fs_read_cb_t read_cb;
    ext4fs_message_cb_t message_cb;

    struct super_block sb;
    struct group_desc *bg;
};

struct ext4fs *ext4fs_new(void *priv)
{
    struct ext4fs *e;

    e = calloc(1, sizeof(*e));
    if (!e)
        return NULL;

    e->priv = priv;

    return e;
}

void ext4fs_del(struct ext4fs *e)
{
    free(e);
}

static void do_read(struct ext4fs *e, uint64_t offs, void *data, uint32_t size)
{
    e->read_cb(e->priv, offs, data, size);
}

static char *arr2str(struct ext4fs *e, void *ptr, int i, int total, int esize)
{
    char *s;
    uint64_t d;

    switch (esize)
    {
    case 1:
        d = *(uint8_t *)(ptr + i * esize);
        break;
    case 2:
        d = *(uint16_t *)(ptr + i * esize);
        break;
    case 4:
        d = *(uint32_t *)(ptr + i * esize);
        break;
    case 8:
        d = *(uint64_t *)(ptr + i * esize);
        break;
    default:
        fatal("unknown esize. %d\n", esize);
    }

    if (i + 1 != total)
    {
        char *n = arr2str(e, ptr, i + 1, total, esize);
        asprintf(&s, "%0*llx %s", esize * 2, (long long)d, n);
        free(n);
    }
    else
        asprintf(&s, "%0*llx", esize * 2, (long long)d);

    return s;
}

static void read_sb(struct ext4fs *e)
{
    if (sizeof(e->sb) != 0x400)
        fatal("sizeof(sb) is not 0x400. %d\n", sizeof(e->sb));
    do_read(e, 0x400, &e->sb, sizeof(e->sb));

#define print_sb_(m) debug("%-28s= 0x%0*llx(%llu)\n", #m, sizeof(e->sb.m) * 2, \
                           (long long)e->sb.m, (long long)e->sb.m)
#define print_sbs(m) debug("%-28s= \"%s\"\n", #m, e->sb.m)
#define print_sba(m)                                         \
    do                                                       \
    {                                                        \
        char *s;                                             \
        s = arr2str(e, e->sb.m,                              \
                    0, sizeof(e->sb.m) / sizeof(e->sb.m[0]), \
                    sizeof(e->sb.m[0]));                     \
        debug("%-28s= %s\n", #m, s);                         \
        free(s);                                             \
    } while (0)
    print_sb_(s_inodes_count);
    print_sb_(s_blocks_count_lo);
    print_sb_(s_r_blocks_count_lo);
    print_sb_(s_free_blocks_count_lo);
    print_sb_(s_free_inodes_count);
    print_sb_(s_first_data_block);
    print_sb_(s_log_block_size);
    print_sb_(s_log_cluster_size);
    print_sb_(s_blocks_per_group);
    print_sb_(s_clusters_per_group);
    print_sb_(s_inodes_per_group);
    print_sb_(s_mtime);
    print_sb_(s_wtime);
    print_sb_(s_mnt_count);
    print_sb_(s_max_mnt_count);
    print_sb_(s_magic);
    print_sb_(s_state);
    print_sb_(s_errors);
    print_sb_(s_minor_rev_level);
    print_sb_(s_lastcheck);
    print_sb_(s_checkinterval);
    print_sb_(s_creator_os);
    print_sb_(s_rev_level);
    print_sb_(s_def_resuid);
    print_sb_(s_def_resgid);
    print_sb_(s_first_ino);
    print_sb_(s_inode_size);
    print_sb_(s_block_group_nr);
    print_sb_(s_feature_compat);
    print_sb_(s_feature_incompat);
    print_sb_(s_feature_ro_compat);
    print_sba(s_uuid);
    print_sbs(s_volume_name);
    print_sbs(s_last_mounted);
    print_sb_(s_algorithm_usage_bitmap);
    print_sb_(s_prealloc_blocks);
    print_sb_(s_prealloc_dir_blocks);
    print_sb_(s_reserved_gdt_blocks);
    print_sba(s_journal_uuid);
    print_sb_(s_journal_inum);
    print_sb_(s_journal_dev);
    print_sb_(s_last_orphan);
    print_sba(s_hash_seed);
    print_sb_(s_def_hash_version);
    print_sb_(s_jnl_backup_type);
    print_sb_(s_desc_size);
    print_sb_(s_default_mount_opts);
    print_sb_(s_first_meta_bg);
    print_sb_(s_mkfs_time);
    print_sba(s_jnl_blocks);
    print_sb_(s_blocks_count_hi);
    print_sb_(s_r_blocks_count_hi);
    print_sb_(s_free_blocks_count_hi);
    print_sb_(s_min_extra_isize);
    print_sb_(s_want_extra_isize);
    print_sb_(s_flags);
    print_sb_(s_raid_stride);
    print_sb_(s_mmp_interval);
    print_sb_(s_mmp_block);
    print_sb_(s_raid_stripe_width);
    print_sb_(s_log_groups_per_flex);
    print_sb_(s_checksum_type);
    print_sb_(s_reserved_pad);
    print_sb_(s_kbytes_written);
    print_sb_(s_snapshot_inum);
    print_sb_(s_snapshot_id);
    print_sb_(s_snapshot_r_blocks_count);
    print_sb_(s_snapshot_list);
    print_sb_(s_error_count);
    print_sb_(s_first_error_time);
    print_sb_(s_first_error_ino);
    print_sb_(s_first_error_block);
    print_sba(s_first_error_func);
    print_sb_(s_first_error_line);
    print_sb_(s_last_error_time);
    print_sb_(s_last_error_ino);
    print_sb_(s_last_error_line);
    print_sb_(s_last_error_block);
    print_sba(s_last_error_func);
    print_sba(s_mount_opts);
    print_sb_(s_usr_quota_inum);
    print_sb_(s_grp_quota_inum);
    print_sb_(s_overhead_blocks);
    print_sba(s_backup_bgs);
    print_sba(s_encrypt_algos);
    print_sba(s_encrypt_pw_salt);
    print_sb_(s_lpf_ino);
    print_sb_(s_prj_quota_inum);
    print_sb_(s_checksum_seed);
    // print_sb_(s_reserved[98]);
    print_sb_(s_checksum);
#undef print_sb_
#undef print_sbs
#undef print_sba
}

static void read_bg(struct ext4fs *e)
{
    int bg_count;
    int i;

    bg_count = e->sb.s_inodes_count / e->sb.s_inodes_per_group;
    debug("block group descriptors %d\n", bg_count);
    e->bg = calloc(bg_count, sizeof(e->bg[0]));
    if (!e->bg)
        fatal("no mem for block group. %d\n", bg_count);

    for (i = 0; i < bg_count; i++)
    {
        do_read(e, 0x400 + sizeof(struct super_block) + sizeof(struct group_desc) * i,
                e->bg + i, sizeof(e->bg[0]));
#define print_bg(m) debug("bg[%d].%-28s= 0x%0*llx(%llu)\n", i, #m, \
                          sizeof(e->bg[i].m) * 2,                  \
                          (long long)e->bg[i].m, (long long)e->bg[i].m)
        print_bg(bg_block_bitmap_lo);
        print_bg(bg_inode_bitmap_lo);
        print_bg(bg_inode_table_lo);
        print_bg(bg_free_blocks_count_lo);
        print_bg(bg_free_inodes_count_lo);
        print_bg(bg_used_dirs_count_lo);
        print_bg(bg_flags);
        print_bg(bg_exclude_bitmap_lo);
        print_bg(bg_block_bitmap_csum_lo);
        print_bg(bg_inode_bitmap_csum_lo);
        print_bg(bg_itable_unused_lo);
        print_bg(bg_checksum);
        print_bg(bg_block_bitmap_hi);
        print_bg(bg_inode_bitmap_hi);
        print_bg(bg_inode_table_hi);
        print_bg(bg_free_blocks_count_hi);
        print_bg(bg_free_inodes_count_hi);
        print_bg(bg_used_dirs_count_hi);
        print_bg(bg_itable_unused_hi);
        print_bg(bg_exclude_bitmap_hi);
        print_bg(bg_block_bitmap_csum_hi);
        print_bg(bg_inode_bitmap_csum_hi);
        print_bg(bg_reserved);

#undef print_bg
    }
}

int ext4fs_load(struct ext4fs *e)
{
    read_sb(e);
    read_bg(e);

    return 0;
}

void ext4fs_set_read_callback(struct ext4fs *e, ext4fs_read_cb_t read_cb)
{
    e->read_cb = read_cb;
}

void ext4fs_set_message_callback(struct ext4fs *e, ext4fs_message_cb_t message_cb)
{
    e->message_cb = message_cb;
}
