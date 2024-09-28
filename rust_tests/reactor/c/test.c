#include <stdlib.h>
#include <stdio.h>

struct Tuple {
    void* subject;
    const char *predicate;
    void* object;
};


void get_query(struct Tuple* t) {
    t->subject = NULL;
    t->predicate = "Hi!";
    t->object = NULL;
}

static const char* lexi = "lexi";
static const char* highlighted = "is highlighted";
static const char* blue = "blue";

struct Tuple* when_handler(struct Tuple* result, size_t *outLen) {
    printf("Called!\n");

    struct Tuple* tuples = (struct Tuple*)malloc(sizeof(struct Tuple));
    *outLen = 1;

    tuples->subject = lexi;
    tuples->predicate = highlighted;
    tuples->object = blue;

    return tuples;
}
