
#define _GNU_SOURCE

#include <sys/syscall.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "ext4.h"

#define fatal(fmt, args...)                      \
    do                                           \
    {                                            \
        _fatal(__func__, __LINE__, fmt, ##args); \
    } while (0)

#define debug(fmt, args...)                        \
    do                                             \
    {                                              \
        _message(__func__, __LINE__, fmt, ##args); \
    } while (0)

struct fsimage
{
    int fd;
    void *priv;
};

static void _fatal(const char *func, int line, const char *fmt, ...)
{
    va_list ap;

    fprintf(stderr, "fatal at %s(), line #%d. errno %s(%d)\n", func, line, strerror(errno), errno);

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    exit(1);
}

static void
read_cb(void *priv, uint64_t offs, void *data, uint32_t size)
{
    struct fsimage *i = priv;
    off_t off;
    ssize_t got;

    off = lseek(i->fd, offs, SEEK_SET);
    if (off == (off_t)-1)
        fatal("lseek() failed. offs %llu\n", offs);

    got = read(i->fd, data, size);
    if (got == -1)
        fatal("read() failed.\n");

    if (got != size)
        fatal("read() failed. got %z, requested %u\n", got, size);
}

static FILE *debug_file;

static void _vmessage(const char *func, int line, const char *format, va_list ap)
{
    char *str = NULL;
    int ret;

    if (!debug_file)
        return;

    ret = vasprintf(&str, format, ap);
    if (ret < 0)
        fatal("vasprintf() failed.\n");

    fprintf(debug_file, "%24s %4d : %s", func, line, str);
    free(str);
}

static void _message(const char *func, int line, const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _vmessage(func, line, format, ap);
    va_end(ap);
}

static void message_cb(void *priv, bool fat, const char *func, int line, const char *format, ...)
{
    va_list ap;

    if (fat)
        debug_file = stderr;

    va_start(ap, format);
    _vmessage(func, line, format, ap);
    va_end(ap);

    if (fat)
        exit(1);
}

static struct ext4fs *e;

int main(int argc, char **argv)
{
    char *opt_debug = NULL;
    char *fs_filename = NULL;

    while (true)
    {
        int opt;

        opt = getopt(argc, argv, "+d:");
        if (opt == -1)
            break;

        switch (opt)
        {
        default:
            fprintf(stderr, "test_ext4 [<options> ...] <ext4-image> [<command> <command-arg> ...]\n"
                            "\n"
                            " options:\n"
                            "   -d <filename>    : filename to save debug messages. \"-\" will print stderr.\n"
                            "\n");
            exit(1);

        case 'd':
            opt_debug = optarg;
            break;
        }
    }

    if (opt_debug)
    {
        if (!strcmp(opt_debug, "-"))
            debug_file = stderr;
        else
        {

            debug_file = fopen(opt_debug, "a");
            if (!debug_file)
                fatal("cannot open file for debug messages. \"%s\"\n", opt_debug);
        }
    }

    if (argv[optind])
        fs_filename = argv[optind++];

    if (!fs_filename)
        fatal("no filesystem filename.\n");
    else
    {
        struct fsimage i = {};

        debug("dump %s\n", fs_filename);

        i.fd = open(fs_filename, O_RDONLY);
        if (i.fd < 0)
            fatal("open(%s) failed.\n", fs_filename);

        i.priv = e = ext4fs_new(&i);
        if (!e)
            fatal("..\n");

        ext4fs_set_message_callback(e, message_cb);
        ext4fs_set_read_callback(e, read_cb);

        ext4fs_load(e);
        ext4fs_command(e, argv + optind);

        ext4fs_del(e);

        close(i.fd);
    }

    return 0;
}
