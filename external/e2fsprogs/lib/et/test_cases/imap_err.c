
#include <stdlib.h>

#define N_(a) a

static const char * const text[] = {
	N_(   "System I/O error"),
	N_(   "Permission denied"),
	N_(   "Over quota"),
	N_(   "Too many user flags in mailbox"),
	N_(   "Mailbox has an invalid format"),
	N_(   "Operation is not supported on mailbox"),
	N_(   "Mailbox does not exist"),
	N_(   "Mailbox already exists"),
	N_(   "Invalid mailbox name"),
	N_(   "Mailbox is locked by POP server"),
	N_(   "Unknown/invalid partition"),
	N_(   "Invalid identifier"),
	N_(   "Message contains NUL characters"),
	N_(   "Message contains bare newlines"),
	N_(   "Message contains non-ASCII characters in headers"),
	N_(   "Message contains invalid header"),
	N_(   "Message has no header/body separator"),
	N_(   "Quota root does not exist"),
	N_(   "Unrecognized character set"),
	N_(   "Invalid user"),
	N_(   "Login incorrect"),
	N_(   "Anonymous login is not permitted"),
	N_(   "Unsupported quota resource"),
	N_(   "Mailbox is over quota"),
	N_(   "Mailbox is at %d%% of quota"),
	N_(   "Message %d no longer exists"),
	N_(   "Unable to checkpoint \\Seen state"),
	N_(   "Unable to preserve \\Seen state"),
	N_(   "LOGOUT received"),
	N_(   "Completed"),
    0
};

struct error_table {
    char const * const * msgs;
    long base;
    int n_msgs;
};
struct et_list {
    struct et_list *next;
    const struct error_table * table;
};
extern struct et_list *_et_list;

const struct error_table et_imap_error_table = { text, -1904809472L, 30 };

static struct et_list link = { 0, 0 };

void initialize_imap_error_table_r(struct et_list **list);
void initialize_imap_error_table(void);

void initialize_imap_error_table(void) {
    initialize_imap_error_table_r(&_et_list);
}

void initialize_imap_error_table_r(struct et_list **list)
{
    struct et_list *et, **end;

    for (end = list, et = *list; et; end = &et->next, et = et->next)
        if (et->table->msgs == text)
            return;
    et = malloc(sizeof(struct et_list));
    if (et == 0) {
        if (!link.table)
            et = &link;
        else
            return;
    }
    et->table = &et_imap_error_table;
    et->next = 0;
    *end = et;
}
