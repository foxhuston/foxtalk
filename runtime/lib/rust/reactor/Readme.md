# The Basics

Foreign handlers communicate through the FFI API:

    constexpr size_t __foxtalk_ipc_buffer_size = 4096;
    uint8_t __foxtalk_ipc_triple_buffer[__foxtalk_ipc_buffer_size];

Triples are laid out as follows:
The first `sizeof(size_t)` bytes are the total length of the buffer used.
This is currently used only as a sanity-check. The next byte is the type
of the first tuple noun:

    enum class NounType : uint8_t {
        Query = 0,
        Symbol = 1,
        CPtr = 2,
        U64 = 3,
        I64 = 4,
        MAX
    } type;

Followed by a variable section of data:
* `Query` has nothing after its marker.
* `Symbol` has a `sizeof(size_t)`-byte length field, followed by
  `length` (character) bytes.
* `CPtr` has a `sizeof(size_t)`-byte field that should be treated as an opaque `u64`.
* `U64` has a `sizeof(uint64_t)`-byte field that should be treated as a `u64`.
* `I64` has a `sizeof(int64_t)`-byte field that should be treated as an `i64`.
