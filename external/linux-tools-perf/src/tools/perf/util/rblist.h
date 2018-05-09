#ifndef __PERF_RBLIST_H
#define __PERF_RBLIST_H

#include <linux/rbtree.h>
#include <stdbool.h>


struct rblist {
	struct rb_root entries;
	unsigned int   nr_entries;

	int (*node_cmp)(struct rb_node *rbn, const void *entry);
	struct rb_node *(*node_new)(struct rblist *rlist, const void *new_entry);
	void (*node_delete)(struct rblist *rblist, struct rb_node *rb_node);
};

void rblist__init(struct rblist *rblist);
void rblist__delete(struct rblist *rblist);
int rblist__add_node(struct rblist *rblist, const void *new_entry);
void rblist__remove_node(struct rblist *rblist, struct rb_node *rb_node);
struct rb_node *rblist__find(struct rblist *rblist, const void *entry);
struct rb_node *rblist__entry(const struct rblist *rblist, unsigned int idx);

static inline bool rblist__empty(const struct rblist *rblist)
{
	return rblist->nr_entries == 0;
}

static inline unsigned int rblist__nr_entries(const struct rblist *rblist)
{
	return rblist->nr_entries;
}

#endif 
