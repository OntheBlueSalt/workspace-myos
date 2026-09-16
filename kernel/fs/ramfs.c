#include "ramfs.h"
#include "../mm/heap.h"
#include "../drivers/screen.h"
#include "../lib/string.h"
#include "../lib/stdint.h"

static fs_node_t *root = 0;

// ---------- 内部工具 ----------

static fs_node_t* alloc_node(const char *name, node_type_t type, fs_node_t *parent) {
    fs_node_t *node = kmalloc(sizeof(fs_node_t));
    if (!node) return 0;
    str_copy(node->name, name, 32);
    node->type = type;
    node->size = 0;
    node->content[0] = '\0';
    node->parent = parent;
    node->children = 0;
    node->next = 0;
    return node;
}

static fs_node_t* find_child(fs_node_t *parent, const char *name) {
    if (!parent || parent->type != NODE_DIR) return 0;
    fs_node_t *child = parent->children;
    while (child) {
        if (str_eq(child->name, name)) return child;
        child = child->next;
    }
    return 0;
}

static void link_child(fs_node_t *parent, fs_node_t *child) {
    child->next = parent->children;
    parent->children = child;
    child->parent = parent;
}

static int split_path(const char *path, char *parent_path, int psize, const char **name) {
    const char *last_slash = 0;
    for (const char *p = path; *p; p++) {
        if (*p == '/') last_slash = p;
    }

    if (!last_slash) {
        str_copy(parent_path, "/", psize);
        *name = path;
    } else if (last_slash == path) {
        str_copy(parent_path, "/", psize);
        *name = last_slash + 1;
    } else {
        int len = last_slash - path;
        if (len >= psize) return -1;
        for (int i = 0; i < len; i++) parent_path[i] = path[i];
        parent_path[len] = '\0';
        *name = last_slash + 1;
    }
    return 0;
}

// ---------- 初始化 ----------

void ramfs_init() {
    root = alloc_node("/", NODE_DIR, 0);
}

fs_node_t* ramfs_root() {
    return root;
}

// ---------- 路径解析 ----------

fs_node_t* ramfs_lookup(const char *path) {
    if (!root) return 0;

    fs_node_t *cur = root;
    const char *p = path;
    if (*p == '/') p++;

    char name[32];
    while (*p) {
        int n = 0;
        while (*p && *p != '/' && n < 31) name[n++] = *p++;
        name[n] = '\0';

        if (str_eq(name, ".")) {
            // 当前目录
        } else if (str_eq(name, "..")) {
            if (cur->parent) cur = cur->parent;
        } else {
            fs_node_t *child = find_child(cur, name);
            if (!child) return 0;
            cur = child;
        }

        if (*p == '/') p++;
    }
    return cur;
}

// ---------- 创建 ----------

int ramfs_mkdir(const char *path) {
    if (ramfs_lookup(path)) return -1;

    char parent_path[64];
    const char *name;
    if (split_path(path, parent_path, 64, &name) < 0) return -1;

    fs_node_t *parent = ramfs_lookup(parent_path);
    if (!parent || parent->type != NODE_DIR) return -1;

    fs_node_t *n = alloc_node(name, NODE_DIR, parent);
    if (!n) return -1;
    link_child(parent, n);
    return 0;
}

int ramfs_create(const char *path) {
    if (ramfs_lookup(path)) return -1;

    char parent_path[64];
    const char *name;
    if (split_path(path, parent_path, 64, &name) < 0) return -1;

    fs_node_t *parent = ramfs_lookup(parent_path);
    if (!parent || parent->type != NODE_DIR) return -1;

    fs_node_t *n = alloc_node(name, NODE_FILE, parent);
    if (!n) return -1;
    link_child(parent, n);
    return 0;
}

// ---------- 读写 ----------

int ramfs_write(const char *path, const char *data) {
    fs_node_t *n = ramfs_lookup(path);
    if (!n || n->type != NODE_FILE) return -1;

    uint32_t i = 0;
    while (data[i] && i < sizeof(n->content) - 1) {
        n->content[i] = data[i];
        i++;
    }
    n->content[i] = '\0';
    n->size = i;
    return 0;
}

int ramfs_read(const char *path, char *buf, uint32_t size) {
    fs_node_t *n = ramfs_lookup(path);
    if (!n || n->type != NODE_FILE) return -1;

    uint32_t i = 0;
    while (i < size - 1 && i < n->size) {
        buf[i] = n->content[i];
        i++;
    }
    buf[i] = '\0';
    return i;
}

// ---------- 删除 ----------

static void unlink_child(fs_node_t *node) {
    fs_node_t *parent = node->parent;
    if (!parent) return;

    fs_node_t *prev = 0;
    fs_node_t *cur = parent->children;
    while (cur) {
        if (cur == node) {
            if (prev) prev->next = cur->next;
            else      parent->children = cur->next;
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

static void free_subtree(fs_node_t *node) {
    if (!node) return;
    fs_node_t *child = node->children;
    while (child) {
        fs_node_t *next = child->next;
        free_subtree(child);
        child = next;
    }
    kfree(node);
}

int ramfs_delete(const char *path) {
    fs_node_t *n = ramfs_lookup(path);
    if (!n || n == root) return -1;

    if (n->type == NODE_DIR && n->children) return -1;

    unlink_child(n);
    kfree(n);
    return 0;
}

// ---------- 列表 ----------

void ramfs_list(const char *path) {
    fs_node_t *dir = path ? ramfs_lookup(path) : root;
    if (!dir || dir->type != NODE_DIR) {
        print_line("not a directory");
        return;
    }
    fs_node_t *p = dir->children;
    if (!p) {
        print_line("(empty)");
        return;
    }
    while (p) {
        if (p->type == NODE_DIR) {
            print_string("[");
            print_string(p->name);
            print_line("]");
        } else {
            print_string(p->name);
            print_string("  ");
            print_dec(p->size);
            print_line(" bytes");
        }
        p = p->next;
    }
}

// ---------- pwd ----------

void ramfs_pwd(fs_node_t *cwd) {
    if (!cwd) return;
    char stack[16][32];
    int depth = 0;
    fs_node_t *p = cwd;
    while (p && p->parent && depth < 16) {
        str_copy(stack[depth++], p->name, 32);
        p = p->parent;
    }
    print_string("/");
    for (int i = depth - 1; i >= 0; i--) {
        print_string(stack[i]);
        if (i) print_string("/");
    }
    print_char('\n');
}

void ramfs_get_path(fs_node_t *node, char *buf, int size)
{
    if (!node) { if (size > 0) buf[0] = '\0'; return; }
    char stack[16][32];
    int depth = 0;
    fs_node_t *p = node;
    while (p && p->parent && depth < 16) {
        str_copy(stack[depth++], p->name, 32);
        p = p->parent;
    }
    int idx = 0;
    if (idx < size - 1) buf[idx++] = '/';
    for (int i = depth - 1; i >= 0; i--) {
        for (int j = 0; stack[i][j] && idx < size - 1; j++)
            buf[idx++] = stack[i][j];
        if (i > 0 && idx < size - 1) buf[idx++] = '/';
    }
    buf[idx] = '\0';
}