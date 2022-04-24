#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

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
    __le32 s_inodes_count;         // 0x0
    __le32 s_blocks_count_lo;      // 0x4
    __le32 s_r_blocks_count_lo;    // 0x8
    __le32 s_free_blocks_count_lo; // 0xC
    __le32 s_free_inodes_count;    // 0x10
    __le32 s_first_data_block;     // 0x14
    __le32 s_log_block_size;       // 0x18
    __le32 s_log_cluster_size;     // 0x1C
    __le32 s_blocks_per_group;     // 0x20
    __le32 s_clusters_per_group;   // 0x24
    __le32 s_inodes_per_group;     // 0x28
    __le32 s_mtime;                // 0x2C
    __le32 s_wtime;                // 0x30
    __le16 s_mnt_count;            // 0x34
    __le16 s_max_mnt_count;        // 0x36
    __le16 s_magic;                // 0x38
    __le16 s_state;                // 0x3A
    __le16 s_errors;               // 0x3C
    __le16 s_minor_rev_level;      // 0x3E
    __le32 s_lastcheck;            // 0x40
    __le32 s_checkinterval;        // 0x44
    __le32 s_creator_os;           // 0x48
    __le32 s_rev_level;            // 0x4C
    __le16 s_def_resuid;           // 0x50
    __le16 s_def_resgid;           // 0x52
    __le32 s_first_ino;            // 0x54
    __le16 s_inode_size;           // 0x58
    __le16 s_block_group_nr;       // 0x5A
    __le32 s_feature_compat;       // 0x5C

#define EXT4_FEATURE_COMPAT_64BIT 0x80
    __le32 s_feature_incompat;       // 0x60
    __le32 s_feature_ro_compat;      // 0x64
    __u8 s_uuid[16];                 // 0x68
    char s_volume_name[16];          // 0x78
    char s_last_mounted[64];         // 0x88
    __le32 s_algorithm_usage_bitmap; // 0xC8
    __u8 s_prealloc_blocks;          // 0xCC
    __u8 s_prealloc_dir_blocks;      // 0xCD
    __le16 s_reserved_gdt_blocks;    // 0xCE
    __u8 s_journal_uuid[16];         // 0xD0
    __le32 s_journal_inum;           // 0xE0
    __le32 s_journal_dev;            // 0xE4
    __le32 s_last_orphan;            // 0xE8
    __le32 s_hash_seed[4];           // 0xEC
    __u8 s_def_hash_version;         // 0xFC
    __u8 s_jnl_backup_type;          // 0xFD
    __le16 s_desc_size;              // 0xFE
    __le32 s_default_mount_opts;     // 0x100
    __le32 s_first_meta_bg;          // 0x104
    __le32 s_mkfs_time;              // 0x108
    __le32 s_jnl_blocks[17];         // 0x10C

    // if 64bit
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

    // if 64bit
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

struct inode
{
#define S_IXOTH 0x1     // Others may execute
#define S_IWOTH 0x2     // Others may write
#define S_IROTH 0x4     // Others may read
#define S_IXGRP 0x8     // Group members may execute
#define S_IWGRP 0x10    // Group members may write
#define S_IRGRP 0x20    // Group members may read
#define S_IXUSR 0x40    // Owner may execute
#define S_IWUSR 0x80    // Owner may write
#define S_IRUSR 0x100   // Owner may read
#define S_ISVTX 0x200   // Sticky bit
#define S_ISGID 0x400   // Set GID
#define S_ISUID 0x800   // Set UID
#define S_IFIFO 0x1000  // FIFO
#define S_IFCHR 0x2000  // Character device
#define S_IFDIR 0x4000  // Directory
#define S_IFBLK 0x6000  // Block device
#define S_IFREG 0x8000  // Regular file
#define S_IFLNK 0xA000  // Symbolic link
#define S_IFSOCK 0xC000 // Socket

    __le16 i_mode;        // 0x0
    __le16 i_uid;         // 0x2
    __le32 i_size_lo;     // 0x4
    __le32 i_atime;       // 0x8
    __le32 i_ctime;       // 0xC
    __le32 i_mtime;       // 0x10
    __le32 i_dtime;       // 0x14
    __le16 i_gid;         // 0x18
    __le16 i_links_count; // 0x1A
    __le32 i_blocks_lo;   // 0x1C

#define EXT4_INDEX_FL 0x1000
#define EXT4_EXTENTS_FL 0x80000
    __le32 i_flags;        // 0x20
    __le32 l_i_version;    // 0x24
    __u8 i_block[60];      // 0x28
    __le32 i_generation;   // 0x64
    __le32 i_file_acl_lo;  // 0x68
    __le32 i_size_hi;      // 0x6C
    __le32 i_obso_faddr;   // 0x70
    __u8 osd2[12];         // 0x74
    __le16 i_extra_isize;  // 0x80
    __le16 i_checksum_hi;  // 0x82
    __le32 i_ctime_extra;  // 0x84
    __le32 i_mtime_extra;  // 0x88
    __le32 i_atime_extra;  // 0x8C
    __le32 i_crtime;       // 0x90
    __le32 i_crtime_extra; // 0x94
    __le32 i_version_hi;   // 0x98
    __le32 i_projid;       // 0x9C
};

struct ext4fs
{
    void *priv;

    ext4fs_read_cb_t read_cb;
    ext4fs_message_cb_t message_cb;

    uint32_t block_size;

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
    if (e->bg)
        free(e->bg);
    free(e);
}

static void do_read(struct ext4fs *e, uint64_t offs, void *data, uint32_t size)
{
    e->read_cb(e->priv, offs, data, size);
}

static char *arr2str(struct ext4fs *e, void *ptr, int i, int total, int esize)
{
    char *s;
    uint64_t d = 0;

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

static bool is_64bit(struct ext4fs *e)
{
    return !!(e->sb.s_feature_incompat & EXT4_FEATURE_COMPAT_64BIT);
}

static void read_sb(struct ext4fs *e)
{
    if (sizeof(e->sb) != 0x400)
        fatal("sizeof(sb) is not 0x400. %d\n", sizeof(e->sb));
    do_read(e, 0x400, &e->sb, sizeof(e->sb));

#define print_sb_(m) debug("(%03x) %-28s= 0x%0*llx(%llu)\n",            \
                           (int)(long)&((struct super_block *)NULL)->m, \
                           #m, sizeof(e->sb.m) * 2,                     \
                           (long long)e->sb.m, (long long)e->sb.m)
#define print_sbs(m) debug("(%03x) %-28s= \"%s\"\n",                       \
                           (int)(long)&((struct super_block *)NULL)->m[0], \
                           #m, e->sb.m)
#define print_sba(m)                                          \
    do                                                        \
    {                                                         \
        char *s;                                              \
        s = arr2str(e, e->sb.m,                               \
                    0, sizeof(e->sb.m) / sizeof(e->sb.m[0]),  \
                    sizeof(e->sb.m[0]));                      \
        debug("(%03x) %-28s= %s\n",                           \
              (int)(long)&((struct super_block *)NULL)->m[0], \
              #m, s);                                         \
        free(s);                                              \
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
    if (is_64bit(e))
    {
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
        print_sbs(s_first_error_func);
        print_sb_(s_first_error_line);
        print_sb_(s_last_error_time);
        print_sb_(s_last_error_ino);
        print_sb_(s_last_error_line);
        print_sb_(s_last_error_block);
        print_sbs(s_last_error_func);
        print_sbs(s_mount_opts);
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
    }
#undef print_sb_
#undef print_sbs
#undef print_sba

    e->block_size = 1 << (10 + e->sb.s_log_block_size);
    debug("block size %u\n", e->block_size);
    debug("inode size %u\n", e->sb.s_inode_size);
    debug("64bit filesystem %d\n", is_64bit(e));
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
        do_read(e,
                (0x400 / e->block_size + 1) * e->block_size +
                    e->sb.s_desc_size * i,
                e->bg + i, e->sb.s_desc_size);

#define print_bg(m) debug("(%02x) bg[%d].%-28s= 0x%0*llx(%llu)\n",    \
                          (int)(long)&((struct group_desc *)NULL)->m, \
                          i, #m,                                      \
                          sizeof(e->bg[i].m) * 2,                     \
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
        if (is_64bit(e))
        {
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
            // print_bg(bg_reserved);
        }

#undef print_bg
    }
}

#define get64(m) ((((uint64_t)m##_hi) << 32) | ((uint64_t)m##_lo))

static uint64_t inode_offset(struct ext4fs *e, uint32_t inode_index)
{
    uint32_t group_index;
    uint32_t index_in_group;
    uint64_t offset;

    inode_index--;
    group_index = inode_index / e->sb.s_inodes_per_group;
    index_in_group = inode_index % e->sb.s_inodes_per_group;

    offset = get64(e->bg[group_index].bg_inode_table) * e->block_size;
    offset += index_in_group * e->sb.s_inode_size;

    return offset;
}

static void read_inode(struct ext4fs *e, uint32_t inode_index, struct inode *inode)
{
    uint64_t offset;
    uint32_t inode_size;

    offset = inode_offset(e, inode_index);
    debug("inode[%d] offset 0x%08llx\n", inode_index, offset);

    inode_size = e->sb.s_inode_size;
    if (inode_size > sizeof(*inode))
        inode_size = sizeof(*inode);

    do_read(e, offset, inode, inode_size);

#define print_i__(m, f) debug("(%02x) inode[%d].%-28s= 0x%0*llx(" f ")\n", \
                              (int)(long)&((struct inode *)NULL)->m,       \
                              inode_index, #m,                             \
                              sizeof(inode->m) * 2,                        \
                              (long long)inode->m, (long long)inode->m)
#define print_i_(m) print_i__(m, "%llu")
#define print_io(m) print_i__(m, "0%llo")
    print_io(i_mode);
    print_i_(i_uid);
    print_i_(i_size_lo);
    print_i_(i_atime);
    print_i_(i_ctime);
    print_i_(i_mtime);
    print_i_(i_dtime);
    print_i_(i_gid);
    print_i_(i_links_count);
    print_i_(i_blocks_lo);
    print_i_(i_flags);
    print_i_(l_i_version);
    print_i_(i_generation);
    print_i_(i_file_acl_lo);
    print_i_(i_size_hi);
    print_i_(i_obso_faddr);
#undef print_i__
#undef print_i_
#undef print_io
}

struct extent_header
{
#define EH_MAGIC 0xF30A
    __le16 eh_magic;
    __le16 eh_entries;
    __le16 eh_max;
    __le16 eh_depth;
    __le32 eh_generation;
};

struct extent_idx
{
    __le32 ei_block;
    __le32 ei_leaf_lo;
    __le16 ei_leaf_hi;
    __le16 ei_unused;
};

struct extent
{
    __le32 ee_block;
    __le16 ee_len;
    __le16 ee_start_hi;
    __le32 ee_start_lo;
};

static void dump_eh(struct ext4fs *e, struct extent_header *eh)
{
#define print_eh(m) debug("(%01x) eh->%-28s= 0x%0*llx(%llu)\n",              \
                          (int)(long)&((struct extent_header *)NULL)->m, #m, \
                          sizeof(eh->m) * 2,                                 \
                          (long long)eh->m, (long long)eh->m)
    print_eh(eh_magic);
    print_eh(eh_entries);
    print_eh(eh_max);
    print_eh(eh_depth);
    print_eh(eh_generation);
#undef print_eh
}

static void dump_ei(struct ext4fs *e, struct extent_idx *ei)
{
#define print_ei(m) debug("(%01x) ee->%-28s= 0x%0*llx(%llu)\n",           \
                          (int)(long)&((struct extent_idx *)NULL)->m, #m, \
                          sizeof(ei->m) * 2,                              \
                          (long long)ei->m, (long long)ei->m)
    print_ei(ei_block);
    print_ei(ei_leaf_lo);
    print_ei(ei_leaf_hi);
#undef print_ee
}

static void dump_ee(struct ext4fs *e, struct extent *ee)
{
#define print_ee(m) debug("(%01x) ee->%-28s= 0x%0*llx(%llu)\n",       \
                          (int)(long)&((struct extent *)NULL)->m, #m, \
                          sizeof(ee->m) * 2,                          \
                          (long long)ee->m, (long long)ee->m)
    print_ee(ee_block);
    print_ee(ee_len);
    print_ee(ee_start_hi);
    print_ee(ee_start_lo);
#undef print_ee
}

static uint64_t _read_data(struct ext4fs *e, struct extent_header *eh,
                           void *data, uint64_t remaining_size)
{
    uint64_t copied = 0;

    dump_eh(e, eh);
    if (eh->eh_magic != EH_MAGIC)
        fatal("wrong eh_magic. 0x%04x\n", eh->eh_magic);

    if (eh->eh_depth == 0)
    {
        struct extent *ee = (void *)&eh[1];
        int i;

        for (i = 0; i < eh->eh_entries; i++)
        {
            uint64_t data_offset;
            uint64_t read_size;

            dump_ee(e, ee);

            read_size = ee->ee_len * e->block_size;
            if (read_size > remaining_size)
                read_size = remaining_size;

            data_offset = get64(ee->ee_start) * e->block_size;
            debug("read data size %llu from 0x%08llx\n", read_size, data_offset);
            do_read(e, data_offset, data + copied, read_size);
            remaining_size -= read_size;
            copied += read_size;

            if (remaining_size == 0)
                break;

            ee++;
        }
    }
    else
    {
        int i;
        struct extent_idx *ei = (void *)&eh[1];
        uint32_t leafbuf[e->block_size / 4];

        for (i = 0; i < eh->eh_entries; i++)
        {
            struct extent_header *leaf_eh;

            dump_ei(e, ei);

            do_read(e, get64(ei->ei_leaf) * e->block_size, leafbuf, e->block_size);

            leaf_eh = (void *)leafbuf;
            copied += _read_data(e, leaf_eh, data + copied, remaining_size);
            remaining_size -= copied;

            ei++;
        }
    }

    return copied;
}

static void *read_inode_data(struct ext4fs *e, uint32_t inode_index, uint64_t *size)
{
    struct inode inode = {};
    void *data;
    uint64_t data_size;

    read_inode(e, inode_index, &inode);

    *size = data_size = get64(inode.i_size);
    data = malloc(data_size);

    if (inode.i_flags & EXT4_INDEX_FL)
        fatal("hashed directory index. not implemented.\n");

    // if extents
    if (inode.i_flags & EXT4_EXTENTS_FL)
    {
        struct extent_header *eh = (void *)&inode.i_block[0];

        if (eh->eh_magic != EH_MAGIC)
            fatal("wrong eh_magic at inode index %u. 0x%04x\n", inode_index, eh->eh_magic);

        _read_data(e, eh, data, data_size);
    }
    else
        fatal("reading non extent inode data is not implemented.\n");

    debug("got data size 0x%08llx, from inode %u\n", data_size, inode_index);

    return data;
}

struct dir_entry
{
    __le32 inode;
    __le16 rec_len;
    __u8 name_len;
    __u8 file_type;
    char name[0];
};

/* each_de() returns
 *  0    : continue for next dir_entry.
 *  != 0 : stop for futher loop.
 */
static void foreach_dir(struct ext4fs *e, uint32_t inode_index,
                        int (*each_de)(struct ext4fs *e, void *priv, struct dir_entry *de),
                        void *priv)
{
    uint64_t i;
    uint64_t size;
    void *data;

    data = read_inode_data(e, inode_index, &size);
    for (i = 0; i < size;)
    {
        struct dir_entry *de = (void *)(data + i);

        if (de->inode)
        {
            debug("de offset %lld\n", i);
            debug("de->inode     0x%08x\n", de->inode);
            debug("de->rec_len   0x%04x\n", de->rec_len);
            debug("de->name_len  0x%02x\n", de->name_len);
            debug("de->file_type 0x%02x\n", de->file_type);
            debug("de->name      \"%s\"\n", de->name);
        }

        if (each_de && each_de(e, priv, de) != 0)
            break;

        if (de->inode == 0 || de->name_len == 0)
            break;

        i += de->rec_len;
    }
    free(data);
}

struct search_inode_priv
{
    char *searching;
    uint32_t inode_index;
};

static int search_inode_index_each_de(struct ext4fs *e, void *priv, struct dir_entry *de)
{
    struct search_inode_priv *search = priv;

    if (de->inode == 0)
        return 0;

    debug("searching \"%s\", this \"%s\"\n", search->searching, de->name);
    if (!strncmp(search->searching, de->name, de->name_len) &&
        search->searching[de->name_len] == 0)
    {
        search->inode_index = de->inode;
        debug("same. inode_index %d\n", de->inode);
        return 1;
    }

    return 0;
}

static uint32_t search_inode_index(struct ext4fs *e, const char *filename)
{
    char *str = strdup(filename);
    char *tok;
    uint32_t inode_index = 2; // start from root inode index

    debug("search inode index for \"%p\"\n", filename);
    debug("search inode index for \"%p\"\n", str);
    debug("search inode index for \"%s\"\n", str);
    tok = strtok(str, "/");
    while (tok)
    {
        struct search_inode_priv search = {};

        debug("tok \"%s\"\n", tok);
        search.searching = tok;
        foreach_dir(e, inode_index, search_inode_index_each_de, &search);

        if (search.inode_index == 0)
            fatal("cannot search \"%s\".\n", tok);

        inode_index = search.inode_index;
        tok = strtok(NULL, "/");
    }

    free(str);
    debug("inode index %d\n", inode_index);
    return inode_index;
}

static void printf_inode(struct ext4fs *e, struct inode *inode, uint32_t inode_index,
                         const char *name, uint32_t name_len)
{
    char ftype;
    char buf[name_len + 1];

    switch (inode->i_mode & 0xf000)
    {
    case S_IFIFO:
        ftype = 'f';
        break;
    case S_IFCHR:
        ftype = 'c';
        break;
    case S_IFDIR:
        ftype = 'd';
        break;
    case S_IFBLK:
        ftype = 'b';
        break;
    case S_IFREG:
        ftype = '-';
        break;
    case S_IFLNK:
        ftype = 'l';
        break;
    case S_IFSOCK:
        ftype = 's';
        break;
    default:
        ftype = '?';
        break;
    }

    strncpy(buf, name, name_len);
    buf[name_len] = 0;

    printf("%7d %c%c%c%c%c%c%c%c%c%c %4d.%-4d %8llu %-s\n",
           inode_index, ftype,
           inode->i_mode & S_IRUSR ? 'r' : '-',
           inode->i_mode & S_IWUSR ? 'w' : '-',
           inode->i_mode & S_IXUSR ? 'x' : '-',
           inode->i_mode & S_IRGRP ? 'r' : '-',
           inode->i_mode & S_IWGRP ? 'w' : '-',
           inode->i_mode & S_IXGRP ? 'x' : '-',
           inode->i_mode & S_IROTH ? 'r' : '-',
           inode->i_mode & S_IWOTH ? 'w' : '-',
           inode->i_mode & S_IXOTH ? 'x' : '-',
           inode->i_uid, inode->i_gid,
           (long long)get64(inode->i_size),
           buf);
}

static int list_each_de(struct ext4fs *e, void *priv, struct dir_entry *de)
{
    struct inode inode = {};

    if (de->inode == 0)
        return 0;

    read_inode(e, de->inode, &inode);
    printf_inode(e, &inode, de->inode, de->name, de->name_len);

    return 0;
}

static int cmd_list(struct ext4fs *e, char **argv)
{
    char *file = argv[0];
    uint32_t inode_index;
    struct inode inode = {};

    if (!file)
        file = "/";
    debug("listing directory. \"%p\"...\n", file);

    inode_index = search_inode_index(e, file);
    debug("listing directory. \"%p\"...\n", file);
    debug("inode index %d\n", inode_index);

    read_inode(e, inode_index, &inode);
    debug("listing directory. \"%p\"...\n", file);
    if (inode.i_mode & S_IFDIR)
    {
        debug("listing directory. \"%p\"...\n", file);
        printf("listing directory. \"%s\"...\n", file);
        foreach_dir(e, inode_index, list_each_de, NULL);
    }
    else
        printf_inode(e, &inode, inode_index, file, strlen(file));

    return 0;
}

static int cmd_cat(struct ext4fs *e, char **argv)
{
    char *file = argv[0];
    void *data;
    uint64_t size;

    if (!file)
        fatal("no file\n");

    data = read_inode_data(e, search_inode_index(e, file), &size);
    write(1, data, size);
    free(data);

    return 0;
}

int ext4fs_load(struct ext4fs *e)
{
    read_sb(e);
    read_bg(e);

    return 0;
}

int ext4fs_command(struct ext4fs *e, char **argv)
{
    if (!argv[0])
        return cmd_list(e, argv);

    if (!strcmp(argv[0], "list"))
        return cmd_list(e, argv + 1);

    if (!strcmp(argv[0], "cat"))
        return cmd_cat(e, argv + 1);

    fatal("unknown command. \"%s\"\n", argv[0]);
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
