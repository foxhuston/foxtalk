
// extern "C" pub fn wish(tuple: Tuple) {

extern "C" {

    struct Tuple {
        void* subject;
        const char *predicate;
        void* object;
    };

    void wish(Tuple t);

    Tuple get_query() {
        return { nullptr, "Hi", nullptr };
    }

    void when_handler(Tuple result) {
        wish(result);
    }
}