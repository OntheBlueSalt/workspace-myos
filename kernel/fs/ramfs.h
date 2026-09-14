#ifndef RAMFS_H
#define RAMFS_H

#include <stdint.h>

typedef enum {
    NODE_FILE,
    NODE_DIR
} node_type_t;

typedef struct fs_node fs_node_t;

struct fs_node {
    char name[32];
    node_type_t type;
    char content[1024];
    uint32_t size;
    fs_node_t *parent;
    fs_node_t *children;
    fs_node_t *next;
};

void        ramfs_init();
fs_node_t*  ramfs_root();

fs_node_t*  ramfs_lookup(const char *path);
int         ramfs_mkdir(const char *path);
int         ramfs_create(const char *path);
int         ramfs_write(const char *path, const char *data);
int         ramfs_read(const char *path, char *buf, uint32_t size);
int         ramfs_delete(const char *path);
void        ramfs_list(const char *path);
void        ramfs_pwd(fs_node_t *cwd);

#endif