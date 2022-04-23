#ifndef __EXT4__H__
#define __EXT4__H__

#include <stdint.h>
#include <stdbool.h>

typedef void (*ext4fs_message_cb_t)(void *priv, bool fat, const char *func, int line, const char *fmt, ...);
typedef void (*ext4fs_read_cb_t)(void *priv, uint64_t offs, void *data, uint32_t size);

struct ext4fs;

struct ext4fs *ext4fs_new(void *priv);
void ext4fs_del(struct ext4fs *e);
void ext4fs_set_read_callback(struct ext4fs *e, ext4fs_read_cb_t read_cb);
void ext4fs_set_message_callback(struct ext4fs *e, ext4fs_message_cb_t message_cb);
int ext4fs_load(struct ext4fs *e);

#endif