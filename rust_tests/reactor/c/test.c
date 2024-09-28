#include <stdlib.h>

struct Tuple {
    void* subject;
    const char *predicate;
    void* object;
};

void wish(struct Tuple t);

void get_query(struct Tuple* t) {
    t->subject = NULL;
    t->predicate = "Hi!";
    t->object = NULL;
}

void when_handler(struct Tuple result) {
    wish(result);
}
