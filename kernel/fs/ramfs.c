#include "ramfs.h"
#include "../mm/heap.h"
#include "../drivers/screen.h"
#include <stdint.h>
#include <stddef.h>


#define NAME_MAX 32
#define CONTENT_MAX 1024

typedef struct file_node file_node_t;

struct file_node
{
    char name[NAME_MAX];
    char content[CONTENT_MAX];
    uint32_t size;
    file_node_t *next;
};

static file_node_t *file_list = NULL;               // 链表头

// 字符串比较
static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == *b;
}

// 字符串复制 (最多 n 字节,包含'\0')
static void str_copy(char *dst, const char *src, int n)
{
    int i = 0;
    while (i < n - 1 && src[i])
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}


// 查找
static file_node_t* find(const char *name)
{
    file_node_t *p = file_list;
    while (p) {
        if (str_eq(p->name, name)) return p;
        p = p->next;
    }
    return NULL;
}

void ramfs_init()
{
    file_list = NULL;
}

int ramfs_create(const char *name)
{
    if (find(name)) return -1;    // 已存在

    file_node_t *node = kmalloc(sizeof(file_node_t));
    if (!node) return -1;

    str_copy(node->name, name, NAME_MAX);
    node->size = 0;
    node->content[0] = '\0';
    node->next = file_list;       // 头插法
    file_list = node;
    return 0;
}

int ramfs_write(const char *name, const char *data)
{
    file_node_t *node = find(name);
    if (!node) return -1;

    uint32_t i = 0;
    while (data[i] && i < CONTENT_MAX - 1) {
        node->content[i] = data[i];
        i++;
    }
    node->content[i] = '\0';
    node->size = i;
    return 0;
}

int ramfs_read(const char *name, char *buf, uint32_t size)
{
    file_node_t *node = find(name);
    if (!node) return -1;

    uint32_t i = 0;
    while (i < size - 1 && i < node->size) {
        buf[i] = node->content[i];
        i++;
    }
    buf[i] = '\0';
    return i;
}

int ramfs_delete(const char *name)
{
    file_node_t *prev = NULL;
    file_node_t *cur = file_list;

    while (cur) {
        if (str_eq(cur->name, name)) {
            if (prev) prev->next = cur->next;
            else      file_list = cur->next;
            kfree(cur);
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}

void ramfs_list()
{
    file_node_t *p = file_list;
    if (!p) {
        print_line("(no files)");
        return;
    }
    while (p) {
        print_string(p->name);
        print_string("  ");
        print_dec(p->size);
        print_line(" bytes");
        p = p->next;
    }
}