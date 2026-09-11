#ifndef RAMFS_H
#define	RAMFS_H

#include <stdint.h>

void ramfs_init();
int ramfs_create(const char *name);                             // 增
int ramfs_write(const char *name, const char *data);            // 改
int ramfs_read(const char *name, char *buf, uint32_t size);     // 查
int ramfs_delete(const char *name);                             // 删
void ramfs_list();

#endif
