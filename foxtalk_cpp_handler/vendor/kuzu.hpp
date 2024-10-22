#pragma once

// Helpers
#if defined _WIN32 || defined __CYGWIN__
#define KUZU_HELPER_DLL_IMPORT __declspec(dllimport)
#define KUZU_HELPER_DLL_EXPORT __declspec(dllexport)
#define KUZU_HELPER_DLL_LOCAL
#define KUZU_HELPER_DEPRECATED __declspec(deprecated)
#else
#define KUZU_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define KUZU_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#define KUZU_HELPER_DLL_LOCAL __attribute__((visibility("hidden")))
#define KUZU_HELPER_DEPRECATED __attribute__((__deprecated__))
#endif

#ifdef KUZU_STATIC_DEFINE
#define KUZU_API
#else
#ifndef KUZU_API
#ifdef KUZU_EXPORTS
/* We are building this library */
#define KUZU_API KUZU_HELPER_DLL_EXPORT
#else
/* We are using this library */
#define KUZU_API KUZU_HELPER_DLL_IMPORT
#endif
#endif
#endif

#ifndef KUZU_DEPRECATED
#define KUZU_DEPRECATED KUZU_HELPER_DEPRECATED
#endif

#ifndef KUZU_DEPRECATED_EXPORT
#define KUZU_DEPRECATED_EXPORT KUZU_API KUZU_DEPRECATED
#endif
#include <memory>
#include <unordered_map>
#include <vector>
// This file defines many macros for controlling copy constructors and move constructors on classes.

// NOLINTBEGIN(bugprone-macro-parentheses): Although this is a good check in general, here, we
// cannot add parantheses around the arguments, for it would be invalid syntax.
#define DELETE_COPY_CONSTRUCT(Object) Object(const Object& other) = delete
#define DELETE_COPY_ASSN(Object) Object& operator=(const Object& other) = delete

#define DELETE_MOVE_CONSTRUCT(Object) Object(Object&& other) = delete
#define DELETE_MOVE_ASSN(Object) Object& operator=(Object&& other) = delete

#define DELETE_BOTH_COPY(Object)                                                                   \
    DELETE_COPY_CONSTRUCT(Object);                                                                 \
    DELETE_COPY_ASSN(Object)

#define DELETE_BOTH_MOVE(Object)                                                                   \
    DELETE_MOVE_CONSTRUCT(Object);                                                                 \
    DELETE_MOVE_ASSN(Object)

#define DEFAULT_MOVE_CONSTRUCT(Object) Object(Object&& other) = default
#define DEFAULT_MOVE_ASSN(Object) Object& operator=(Object&& other) = default

#define DEFAULT_BOTH_MOVE(Object)                                                                  \
    DEFAULT_MOVE_CONSTRUCT(Object);                                                                \
    DEFAULT_MOVE_ASSN(Object)

#define EXPLICIT_COPY_METHOD(Object)                                                               \
    Object copy() const {                                                                          \
        return *this;                                                                              \
    }

// EXPLICIT_COPY_DEFAULT_MOVE should be the default choice. It expects a PRIVATE copy constructor to
// be defined, which will be used by an explicit `copy()` method. For instance:
//
//   private:
//     MyClass(const MyClass& other) : field(other.field.copy()) {}
//
//   public:
//     EXPLICIT_COPY_DEFAULT_MOVE(MyClass);
//
// Now:
//
// MyClass o1;
// MyClass o2 = o1; // Compile error, copy assignment deleted.
// MyClass o2 = o1.copy(); // OK.
// MyClass o2(o1); // Compile error, copy constructor is private.
#define EXPLICIT_COPY_DEFAULT_MOVE(Object)                                                         \
    DELETE_COPY_ASSN(Object);                                                                      \
    DEFAULT_BOTH_MOVE(Object);                                                                     \
    EXPLICIT_COPY_METHOD(Object)

// NO_COPY should be used for objects that for whatever reason, should never be copied, but can be
// moved.
#define DELETE_COPY_DEFAULT_MOVE(Object)                                                           \
    DELETE_BOTH_COPY(Object);                                                                      \
    DEFAULT_BOTH_MOVE(Object)

// NO_MOVE_OR_COPY exists solely for explicitness, when an object cannot be moved nor copied. Any
// object containing a lock cannot be moved or copied.
#define DELETE_COPY_AND_MOVE(Object)                                                               \
    DELETE_BOTH_COPY(Object);                                                                      \
    DELETE_BOTH_MOVE(Object)
// NOLINTEND(bugprone-macro-parentheses):

template<typename T>
static std::vector<T> copyVector(const std::vector<T>& objects) {
    std::vector<T> result;
    result.reserve(objects.size());
    for (auto& object : objects) {
        result.push_back(object.copy());
    }
    return result;
}

template<typename T>
static std::vector<std::shared_ptr<T>> copyVector(const std::vector<std::shared_ptr<T>>& objects) {
    std::vector<std::shared_ptr<T>> result;
    result.reserve(objects.size());
    for (auto& object : objects) {
        T& ob = *object;
        result.push_back(ob.copy());
    }
    return result;
}

template<typename T>
static std::vector<std::unique_ptr<T>> copyVector(const std::vector<std::unique_ptr<T>>& objects) {
    std::vector<std::unique_ptr<T>> result;
    result.reserve(objects.size());
    for (auto& object : objects) {
        T& ob = *object;
        result.push_back(ob.copy());
    }
    return result;
}

// TODO: remove
template<typename T>
static std::vector<std::unique_ptr<T>> cloneVector(const std::vector<std::unique_ptr<T>>& objects) {
    std::vector<std::unique_ptr<T>> result;
    result.reserve(objects.size());
    for (auto& object : objects) {
        T& ob = *object;
        result.push_back(ob.clone());
    }
    return result;
}

template<typename K, typename V>
static std::unordered_map<K, V> copyMap(const std::unordered_map<K, V>& objects) {
    std::unordered_map<K, V> result;
    for (auto& [k, v] : objects) {
        result.insert({k, v.copy()});
    }
    return result;
}

#include <stdint.h>

namespace kuzu {
namespace common {

/**
 * @brief Interface for displaying progress of a pipeline and a query.
 */
class ProgressBarDisplay {
public:
    ProgressBarDisplay() : pipelineProgress{0}, numPipelines{0}, numPipelinesFinished{0} {};

    virtual ~ProgressBarDisplay() = default;

    // Update the progress of the pipeline and the number of finished pipelines. queryID is used to
    // identify the query when we track progress of multiple queries asynchronously
    virtual void updateProgress(uint64_t queryID, double newPipelineProgress,
        uint32_t newNumPipelinesFinished) = 0;

    // Finish the progress display. queryID is used to identify the query when we track progress of
    // multiple queries asynchronously
    virtual void finishProgress(uint64_t queryID) = 0;

    void setNumPipelines(uint32_t newNumPipelines) { numPipelines = newNumPipelines; };

protected:
    double pipelineProgress;
    uint32_t numPipelines;
    uint32_t numPipelinesFinished;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string_view>

namespace kuzu {
namespace common {

extern const char* KUZU_VERSION;

constexpr uint64_t DEFAULT_VECTOR_CAPACITY_LOG_2 = 11;
constexpr uint64_t DEFAULT_VECTOR_CAPACITY = static_cast<uint64_t>(1)
                                             << DEFAULT_VECTOR_CAPACITY_LOG_2;

constexpr double DEFAULT_HT_LOAD_FACTOR = 1.5;

// This is the default thread sleep time we use when a thread,
// e.g., a worker thread is in TaskScheduler, needs to block.
constexpr uint64_t THREAD_SLEEP_TIME_WHEN_WAITING_IN_MICROS = 500;

constexpr uint64_t DEFAULT_CHECKPOINT_WAIT_TIMEOUT_IN_MICROS = 5000000;

// Note that some places use std::bit_ceil to calculate resizes,
// which won't work for values other than 2. If this is changed, those will need to be updated
constexpr uint64_t CHUNK_RESIZE_RATIO = 2;

struct InternalKeyword {
    static constexpr char ANONYMOUS[] = "";
    static constexpr char ID[] = "_ID";
    static constexpr char LABEL[] = "_LABEL";
    static constexpr char SRC[] = "_SRC";
    static constexpr char DST[] = "_DST";
    static constexpr char DIRECTION[] = "_DIRECTION";
    static constexpr char LENGTH[] = "_LENGTH";
    static constexpr char NODES[] = "_NODES";
    static constexpr char RELS[] = "_RELS";
    static constexpr char STAR[] = "*";
    static constexpr char PLACE_HOLDER[] = "_PLACE_HOLDER";
    static constexpr char MAP_KEY[] = "KEY";
    static constexpr char MAP_VALUE[] = "VALUE";

    static constexpr std::string_view ROW_OFFSET = "_row_offset";
    static constexpr std::string_view SRC_OFFSET = "_src_offset";
    static constexpr std::string_view DST_OFFSET = "_dst_offset";
};

enum PageSizeClass : uint8_t {
    PAGE_4KB = 0,
    PAGE_256KB = 1,
};

// Currently the system supports files with 2 different pages size, which we refer to as
// PAGE_4KB_SIZE and PAGE_256KB_SIZE. PAGE_4KB_SIZE is the default size of the page which is the
// unit of read/write to the database files, such as to store columns or lists. For now, this value
// cannot be changed. But technically it can change from 2^12 to 2^16. 2^12 lower bound is assuming
// the OS page size is 4K. 2^16 is because currently we leave 11 fixed number of bits for
// relOffInPage and the maximum number of bytes needed for an edge is 20 bytes so 11 + log_2(20)
// = 15.xxx, so certainly over 2^16-size pages, we cannot utilize the page for storing adjacency
// lists.
struct BufferPoolConstants {
    static constexpr uint64_t PAGE_4KB_SIZE_LOG2 = 12;
    static constexpr uint64_t PAGE_4KB_SIZE = static_cast<uint64_t>(1) << PAGE_4KB_SIZE_LOG2;
    // Page size for files with large pages, e.g., temporary files that are used by operators that
    // may require large amounts of memory.
    static constexpr uint64_t PAGE_256KB_SIZE_LOG2 = 18;
    static constexpr uint64_t PAGE_256KB_SIZE = static_cast<uint64_t>(1) << PAGE_256KB_SIZE_LOG2;
    // If a user does not specify a max size for BM, we by default set the max size of BM to
    // maxPhyMemSize * DEFAULT_PHY_MEM_SIZE_RATIO_FOR_BM.
    static constexpr double DEFAULT_PHY_MEM_SIZE_RATIO_FOR_BM = 0.8;
    // For each PURGE_EVICTION_QUEUE_INTERVAL candidates added to the eviction queue, we will call
    // `removeNonEvictableCandidates` to remove candidates that are not evictable. See
    // `EvictionQueue::removeNonEvictableCandidates()` for more details.
    static constexpr uint64_t EVICTION_QUEUE_PURGING_INTERVAL = 1024;
// The default max size for a VMRegion.
#ifdef __32BIT__
    static constexpr uint64_t DEFAULT_VM_REGION_MAX_SIZE = (uint64_t)1 << 30; // (1GB)
#else
    static constexpr uint64_t DEFAULT_VM_REGION_MAX_SIZE = static_cast<uint64_t>(1) << 43; // (8TB)
#endif

    static constexpr uint64_t DEFAULT_BUFFER_POOL_SIZE_FOR_TESTING = 1ull << 26; // (64MB)
};

struct StorageConstants {
    static constexpr char OVERFLOW_FILE_SUFFIX[] = ".ovf";
    static constexpr char WAL_FILE_SUFFIX[] = ".wal";
    static constexpr char SHADOWING_SUFFIX[] = ".shadow";
    static constexpr char INDEX_FILE_SUFFIX[] = ".hindex";
    static constexpr char CATALOG_FILE_NAME[] = "catalog.kz";
    static constexpr char CATALOG_FILE_NAME_FOR_WAL[] = "catalog.shadow";
    static constexpr char DATA_FILE_NAME[] = "data.kz";
    static constexpr char METADATA_FILE_NAME[] = "metadata.kz";
    static constexpr char METADATA_FILE_NAME_FOR_WAL[] = "metadata.shadow";
    static constexpr char LOCK_FILE_NAME[] = ".lock";

    // The number of pages that we add at one time when we need to grow a file.
    static constexpr uint64_t PAGE_GROUP_SIZE_LOG2 = 10;
    static constexpr uint64_t PAGE_GROUP_SIZE = static_cast<uint64_t>(1) << PAGE_GROUP_SIZE_LOG2;
    static constexpr uint64_t PAGE_IDX_IN_GROUP_MASK =
        (static_cast<uint64_t>(1) << PAGE_GROUP_SIZE_LOG2) - 1;

    static constexpr uint64_t NODE_GROUP_SIZE_LOG2 = 17; // 64 * 2048 nodes per group
    static constexpr uint64_t NODE_GROUP_SIZE = static_cast<uint64_t>(1) << NODE_GROUP_SIZE_LOG2;
    static constexpr uint64_t NUM_VECTORS_PER_NODE_GROUP =
        NODE_GROUP_SIZE / DEFAULT_VECTOR_CAPACITY;

    static constexpr double PACKED_CSR_DENSITY = 0.8;
    static constexpr double LEAF_LOW_CSR_DENSITY = 0.1;
    static constexpr double LEAF_HIGH_CSR_DENSITY = 1.0;
    // The number of CSR lists in a leaf region.
    static constexpr uint64_t CSR_LEAF_REGION_SIZE_LOG2 = 10;
    static constexpr uint64_t CSR_LEAF_REGION_SIZE = static_cast<uint64_t>(1)
                                                     << CSR_LEAF_REGION_SIZE_LOG2;

    static constexpr uint64_t MAX_NUM_ROWS_IN_TABLE = static_cast<uint64_t>(1) << 62;
};

// Hash Index Configurations
struct HashIndexConstants {
    static constexpr uint16_t SLOT_CAPACITY_BYTES = 256;
    static constexpr double MAX_LOAD_FACTOR = 0.8;
};

struct CopyConstants {
    // Initial size of buffer for CSV Reader.
    static constexpr uint64_t INITIAL_BUFFER_SIZE = 16384;
    // This means that we will usually read the entirety of the contents of the file we need for a
    // block in one read request. It is also very small, which means we can parallelize small files
    // efficiently.
    static constexpr uint64_t PARALLEL_BLOCK_SIZE = INITIAL_BUFFER_SIZE / 2;

    static constexpr const char* BOOL_CSV_PARSING_OPTIONS[] = {"HEADER", "PARALLEL"};
    static constexpr bool DEFAULT_CSV_HAS_HEADER = false;
    static constexpr bool DEFAULT_CSV_PARALLEL = true;

    // Default configuration for csv file parsing
    static constexpr const char* STRING_CSV_PARSING_OPTIONS[] = {"ESCAPE", "DELIM", "QUOTE"};
    static constexpr char DEFAULT_CSV_ESCAPE_CHAR = '"';
    static constexpr char DEFAULT_CSV_DELIMITER = ',';
    static constexpr char DEFAULT_CSV_QUOTE_CHAR = '"';
    static constexpr char DEFAULT_CSV_LIST_BEGIN_CHAR = '[';
    static constexpr char DEFAULT_CSV_LIST_END_CHAR = ']';
    static constexpr char DEFAULT_CSV_LINE_BREAK = '\n';
    static constexpr const char* ROW_IDX_COLUMN_NAME = "ROW_IDX";
    static constexpr uint64_t PANDAS_PARTITION_COUNT = 50 * DEFAULT_VECTOR_CAPACITY;

    static constexpr const char* INT_CSV_PARSING_OPTIONS[] = {"SKIP"};
    static constexpr uint64_t DEFAULT_CSV_SKIP_NUM = 0;
};

struct RdfConstants {
    static constexpr char IN_MEMORY_OPTION[] = "IN_MEMORY";
    static constexpr char STRICT_OPTION[] = "STRICT";
};

struct PlannerKnobs {
    static constexpr double NON_EQUALITY_PREDICATE_SELECTIVITY = 0.1;
    static constexpr double EQUALITY_PREDICATE_SELECTIVITY = 0.01;
    static constexpr uint64_t BUILD_PENALTY = 2;
    // Avoid doing probe to build SIP if we have to accumulate a probe side that is much bigger than
    // build side. Also avoid doing build to probe SIP if probe side is not much bigger than build.
    static constexpr uint64_t SIP_RATIO = 5;
};

struct OrderByConstants {
    static constexpr uint64_t NUM_BYTES_FOR_PAYLOAD_IDX = 8;
    static constexpr uint64_t MIN_SIZE_TO_REDUCE = common::DEFAULT_VECTOR_CAPACITY * 5;
    static constexpr uint64_t MIN_LIMIT_RATIO_TO_REDUCE = 2;
};

struct ParquetConstants {
    static constexpr uint64_t PARQUET_DEFINE_VALID = 65535;
    static constexpr const char* PARQUET_MAGIC_WORDS = "PAR1";
    // We limit the uncompressed page size to 100MB.
    // The max size in Parquet is 2GB, but we choose a more conservative limit.
    static constexpr uint64_t MAX_UNCOMPRESSED_PAGE_SIZE = 100000000;
    // Dictionary pages must be below 2GB. Unlike data pages, there's only one dictionary page.
    // For this reason we go with a much higher, but still a conservative upper bound of 1GB.
    static constexpr uint64_t MAX_UNCOMPRESSED_DICT_PAGE_SIZE = 1e9;
    // The maximum size a key entry in an RLE page takes.
    static constexpr uint64_t MAX_DICTIONARY_KEY_SIZE = sizeof(uint32_t);
    // The size of encoding the string length.
    static constexpr uint64_t STRING_LENGTH_SIZE = sizeof(uint32_t);
    static constexpr uint64_t MAX_STRING_STATISTICS_SIZE = 10000;
    static constexpr uint64_t PARQUET_INTERVAL_SIZE = 12;
    static constexpr uint64_t PARQUET_UUID_SIZE = 16;
};

struct ExportCSVConstants {
    static constexpr const char* DEFAULT_CSV_NEWLINE = "\n";
    static constexpr const char* DEFAULT_NULL_STR = "";
    static constexpr bool DEFAULT_FORCE_QUOTE = false;
    static constexpr uint64_t DEFAULT_CSV_FLUSH_SIZE = 4096 * 8;
};

struct ImportDBConstants {
    static constexpr char SCHEMA_NAME[] = "schema.cypher";
    static constexpr char COPY_NAME[] = "copy.cypher";
};

static constexpr char ATTACHED_KUZU_DB_TYPE[] = "KUZU";

static constexpr char LOCAL_DB_NAME[] = "local(kuzu)";

constexpr auto DECIMAL_PRECISION_LIMIT = 38;

static constexpr char SCAN_JSON_FUNC_NAME[] = "READ_JSON";

} // namespace common
} // namespace kuzu

#include <memory>

#include <span>

namespace kuzu {
namespace common {

class ArrowNullMaskTree;
class Serializer;
class Deserializer;

constexpr uint64_t NULL_BITMASKS_WITH_SINGLE_ONE[64] = {0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800, 0x1000, 0x2000, 0x4000, 0x8000, 0x10000, 0x20000, 0x40000, 0x80000,
    0x100000, 0x200000, 0x400000, 0x800000, 0x1000000, 0x2000000, 0x4000000, 0x8000000, 0x10000000,
    0x20000000, 0x40000000, 0x80000000, 0x100000000, 0x200000000, 0x400000000, 0x800000000,
    0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000, 0x10000000000, 0x20000000000,
    0x40000000000, 0x80000000000, 0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
    0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000, 0x10000000000000,
    0x20000000000000, 0x40000000000000, 0x80000000000000, 0x100000000000000, 0x200000000000000,
    0x400000000000000, 0x800000000000000, 0x1000000000000000, 0x2000000000000000,
    0x4000000000000000, 0x8000000000000000};
constexpr uint64_t NULL_BITMASKS_WITH_SINGLE_ZERO[64] = {0xfffffffffffffffe, 0xfffffffffffffffd,
    0xfffffffffffffffb, 0xfffffffffffffff7, 0xffffffffffffffef, 0xffffffffffffffdf,
    0xffffffffffffffbf, 0xffffffffffffff7f, 0xfffffffffffffeff, 0xfffffffffffffdff,
    0xfffffffffffffbff, 0xfffffffffffff7ff, 0xffffffffffffefff, 0xffffffffffffdfff,
    0xffffffffffffbfff, 0xffffffffffff7fff, 0xfffffffffffeffff, 0xfffffffffffdffff,
    0xfffffffffffbffff, 0xfffffffffff7ffff, 0xffffffffffefffff, 0xffffffffffdfffff,
    0xffffffffffbfffff, 0xffffffffff7fffff, 0xfffffffffeffffff, 0xfffffffffdffffff,
    0xfffffffffbffffff, 0xfffffffff7ffffff, 0xffffffffefffffff, 0xffffffffdfffffff,
    0xffffffffbfffffff, 0xffffffff7fffffff, 0xfffffffeffffffff, 0xfffffffdffffffff,
    0xfffffffbffffffff, 0xfffffff7ffffffff, 0xffffffefffffffff, 0xffffffdfffffffff,
    0xffffffbfffffffff, 0xffffff7fffffffff, 0xfffffeffffffffff, 0xfffffdffffffffff,
    0xfffffbffffffffff, 0xfffff7ffffffffff, 0xffffefffffffffff, 0xffffdfffffffffff,
    0xffffbfffffffffff, 0xffff7fffffffffff, 0xfffeffffffffffff, 0xfffdffffffffffff,
    0xfffbffffffffffff, 0xfff7ffffffffffff, 0xffefffffffffffff, 0xffdfffffffffffff,
    0xffbfffffffffffff, 0xff7fffffffffffff, 0xfeffffffffffffff, 0xfdffffffffffffff,
    0xfbffffffffffffff, 0xf7ffffffffffffff, 0xefffffffffffffff, 0xdfffffffffffffff,
    0xbfffffffffffffff, 0x7fffffffffffffff};

const uint64_t NULL_LOWER_MASKS[65] = {0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff, 0x1ff,
    0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff, 0x1ffff, 0x3ffff, 0x7ffff, 0xfffff,
    0x1fffff, 0x3fffff, 0x7fffff, 0xffffff, 0x1ffffff, 0x3ffffff, 0x7ffffff, 0xfffffff, 0x1fffffff,
    0x3fffffff, 0x7fffffff, 0xffffffff, 0x1ffffffff, 0x3ffffffff, 0x7ffffffff, 0xfffffffff,
    0x1fffffffff, 0x3fffffffff, 0x7fffffffff, 0xffffffffff, 0x1ffffffffff, 0x3ffffffffff,
    0x7ffffffffff, 0xfffffffffff, 0x1fffffffffff, 0x3fffffffffff, 0x7fffffffffff, 0xffffffffffff,
    0x1ffffffffffff, 0x3ffffffffffff, 0x7ffffffffffff, 0xfffffffffffff, 0x1fffffffffffff,
    0x3fffffffffffff, 0x7fffffffffffff, 0xffffffffffffff, 0x1ffffffffffffff, 0x3ffffffffffffff,
    0x7ffffffffffffff, 0xfffffffffffffff, 0x1fffffffffffffff, 0x3fffffffffffffff,
    0x7fffffffffffffff, 0xffffffffffffffff};
const uint64_t NULL_HIGH_MASKS[65] = {0x0, 0x8000000000000000, 0xc000000000000000,
    0xe000000000000000, 0xf000000000000000, 0xf800000000000000, 0xfc00000000000000,
    0xfe00000000000000, 0xff00000000000000, 0xff80000000000000, 0xffc0000000000000,
    0xffe0000000000000, 0xfff0000000000000, 0xfff8000000000000, 0xfffc000000000000,
    0xfffe000000000000, 0xffff000000000000, 0xffff800000000000, 0xffffc00000000000,
    0xffffe00000000000, 0xfffff00000000000, 0xfffff80000000000, 0xfffffc0000000000,
    0xfffffe0000000000, 0xffffff0000000000, 0xffffff8000000000, 0xffffffc000000000,
    0xffffffe000000000, 0xfffffff000000000, 0xfffffff800000000, 0xfffffffc00000000,
    0xfffffffe00000000, 0xffffffff00000000, 0xffffffff80000000, 0xffffffffc0000000,
    0xffffffffe0000000, 0xfffffffff0000000, 0xfffffffff8000000, 0xfffffffffc000000,
    0xfffffffffe000000, 0xffffffffff000000, 0xffffffffff800000, 0xffffffffffc00000,
    0xffffffffffe00000, 0xfffffffffff00000, 0xfffffffffff80000, 0xfffffffffffc0000,
    0xfffffffffffe0000, 0xffffffffffff0000, 0xffffffffffff8000, 0xffffffffffffc000,
    0xffffffffffffe000, 0xfffffffffffff000, 0xfffffffffffff800, 0xfffffffffffffc00,
    0xfffffffffffffe00, 0xffffffffffffff00, 0xffffffffffffff80, 0xffffffffffffffc0,
    0xffffffffffffffe0, 0xfffffffffffffff0, 0xfffffffffffffff8, 0xfffffffffffffffc,
    0xfffffffffffffffe, 0xffffffffffffffff};

class NullMask {
public:
    static constexpr uint64_t NO_NULL_ENTRY = 0;
    static constexpr uint64_t ALL_NULL_ENTRY = ~uint64_t(NO_NULL_ENTRY);
    static constexpr uint64_t NUM_BITS_PER_NULL_ENTRY_LOG2 = 6;
    static constexpr uint64_t NUM_BITS_PER_NULL_ENTRY = (uint64_t)1 << NUM_BITS_PER_NULL_ENTRY_LOG2;
    static constexpr uint64_t NUM_BYTES_PER_NULL_ENTRY = NUM_BITS_PER_NULL_ENTRY >> 3;

    // For creating a managed null mask
    explicit NullMask(uint64_t capacity) : mayContainNulls{false} {
        auto numNullEntries = (capacity + NUM_BITS_PER_NULL_ENTRY - 1) / NUM_BITS_PER_NULL_ENTRY;
        buffer = std::make_unique<uint64_t[]>(numNullEntries);
        data = std::span(buffer.get(), numNullEntries);
        std::fill(data.begin(), data.end(), NO_NULL_ENTRY);
    }

    // For creating a null mask using existing data
    explicit NullMask(std::span<uint64_t> nullData, bool mayContainNulls)
        : data{nullData}, buffer{}, mayContainNulls{mayContainNulls} {}

    inline void setAllNonNull() {
        if (!mayContainNulls) {
            return;
        }
        std::fill(data.begin(), data.end(), NO_NULL_ENTRY);
        mayContainNulls = false;
    }
    inline void setAllNull() {
        std::fill(data.begin(), data.end(), ALL_NULL_ENTRY);
        mayContainNulls = true;
    }

    inline bool hasNoNullsGuarantee() const { return !mayContainNulls; }

    static void setNull(uint64_t* nullEntries, uint32_t pos, bool isNull);
    inline void setNull(uint32_t pos, bool isNull) {
        setNull(data.data(), pos, isNull);
        if (isNull) {
            mayContainNulls = true;
        }
    }

    static inline bool isNull(const uint64_t* nullEntries, uint32_t pos) {
        auto [entryPos, bitPosInEntry] = getNullEntryAndBitPos(pos);
        return nullEntries[entryPos] & NULL_BITMASKS_WITH_SINGLE_ONE[bitPosInEntry];
    }

    inline bool isNull(uint32_t pos) const { return isNull(data.data(), pos); }

    // const because updates to the data must set mayContainNulls if any value
    // becomes non-null
    // Modifying the underlying data should be done with setNull or copyFromNullData
    inline const uint64_t* getData() const { return data.data(); }

    static inline uint64_t getNumNullEntries(uint64_t numNullBits) {
        return (numNullBits >> NUM_BITS_PER_NULL_ENTRY_LOG2) +
               ((numNullBits - (numNullBits << NUM_BITS_PER_NULL_ENTRY_LOG2)) == 0 ? 0 : 1);
    }

    // Copies bitpacked null flags from one buffer to another, starting at an arbitrary bit
    // offset and preserving adjacent bits.
    //
    // returns true if we have copied a nullBit with value 1 (indicates a null value) to
    // dstNullEntries.
    static bool copyNullMask(const uint64_t* srcNullEntries, uint64_t srcOffset,
        uint64_t* dstNullEntries, uint64_t dstOffset, uint64_t numBitsToCopy, bool invert = false);

    inline bool copyFrom(const NullMask& nullMask, uint64_t srcOffset, uint64_t dstOffset,
        uint64_t numBitsToCopy, bool invert = false) {
        if (nullMask.hasNoNullsGuarantee()) {
            setNullFromRange(dstOffset, numBitsToCopy, invert);
            return invert;
        } else {
            return copyFromNullBits(nullMask.getData(), srcOffset, dstOffset, numBitsToCopy,
                invert);
        }
    }
    bool copyFromNullBits(const uint64_t* srcNullEntries, uint64_t srcOffset, uint64_t dstOffset,
        uint64_t numBitsToCopy, bool invert = false);

    // Sets the given number of bits to null (if isNull is true) or non-null (if isNull is false),
    // starting at the offset
    static void setNullRange(uint64_t* nullEntries, uint64_t offset, uint64_t numBitsToSet,
        bool isNull);

    void setNullFromRange(uint64_t offset, uint64_t numBitsToSet, bool isNull);

    void resize(uint64_t capacity);

    void operator|=(const NullMask& other);

    // Fast calculation of the minimum and maximum null values
    // (essentially just three states, all null, all non-null and some null)
    static std::pair<bool, bool> getMinMax(const uint64_t* nullEntries, uint64_t numValues);

private:
    static inline std::pair<uint64_t, uint64_t> getNullEntryAndBitPos(uint64_t pos) {
        auto nullEntryPos = pos >> NUM_BITS_PER_NULL_ENTRY_LOG2;
        return std::make_pair(nullEntryPos,
            pos - (nullEntryPos << NullMask::NUM_BITS_PER_NULL_ENTRY_LOG2));
    }

    static bool copyUnaligned(const uint64_t* srcNullEntries, uint64_t srcOffset,
        uint64_t* dstNullEntries, uint64_t dstOffset, uint64_t numBitsToCopy, bool invert = false);

private:
    std::span<uint64_t> data;
    std::unique_ptr<uint64_t[]> buffer;
    bool mayContainNulls;
};

} // namespace common
} // namespace kuzu

#include <iterator>
#include <memory>
#include <vector>

namespace kuzu {
namespace storage {
class MemoryBuffer;
class MemoryManager;
} // namespace storage

namespace common {

struct BufferBlock {
public:
    explicit BufferBlock(std::unique_ptr<storage::MemoryBuffer> block);
    ~BufferBlock();

    uint64_t size() const;
    uint8_t* data() const;

public:
    uint64_t currentOffset;
    std::unique_ptr<storage::MemoryBuffer> block;

    void resetCurrentOffset() { currentOffset = 0; }
};

class InMemOverflowBuffer {

public:
    explicit InMemOverflowBuffer(storage::MemoryManager* memoryManager)
        : memoryManager{memoryManager}, currentBlock{nullptr} {};

    uint8_t* allocateSpace(uint64_t size);

    void merge(InMemOverflowBuffer& other) {
        move(begin(other.blocks), end(other.blocks), back_inserter(blocks));
        // We clear the other InMemOverflowBuffer's block because when it is deconstructed,
        // InMemOverflowBuffer's deconstructed tries to free these pages by calling
        // memoryManager->freeBlock, but it should not because this InMemOverflowBuffer still
        // needs them.
        other.blocks.clear();
        currentBlock = other.currentBlock;
    }

    // Releases all memory accumulated for string overflows so far and re-initializes its state to
    // an empty buffer. If there is a large string that used point to any of these overflow buffers
    // they will error.
    void resetBuffer();

private:
    bool requireNewBlock(uint64_t sizeToAllocate) {
        return currentBlock == nullptr ||
               (currentBlock->currentOffset + sizeToAllocate) > currentBlock->size();
    }

    void allocateNewBlock(uint64_t size);

private:
    std::vector<std::unique_ptr<BufferBlock>> blocks;
    storage::MemoryManager* memoryManager;
    BufferBlock* currentBlock;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>

namespace kuzu {
namespace common {

enum class ExpressionType : uint8_t {
    // Boolean Connection Expressions
    OR = 0,
    XOR = 1,
    AND = 2,
    NOT = 3,

    // Comparison Expressions
    EQUALS = 10,
    NOT_EQUALS = 11,
    GREATER_THAN = 12,
    GREATER_THAN_EQUALS = 13,
    LESS_THAN = 14,
    LESS_THAN_EQUALS = 15,

    // Null Operator Expressions
    IS_NULL = 50,
    IS_NOT_NULL = 51,

    PROPERTY = 60,

    LITERAL = 70,

    STAR = 80,

    VARIABLE = 90,
    PATH = 91,
    PATTERN = 92, // Node & Rel pattern

    PARAMETER = 100,

    FUNCTION = 110,

    AGGREGATE_FUNCTION = 130,

    SUBQUERY = 190,

    CASE_ELSE = 200,

    GRAPH = 210,

    LAMBDA = 220,

    // NOTE: this enum has type uint8_t so don't assign over 255.
};

struct ExpressionTypeUtil {
    static bool isUnary(ExpressionType type);
    static bool isBinary(ExpressionType type);
    static bool isBoolean(ExpressionType type);
    static bool isComparison(ExpressionType type);
    static bool isNullOperator(ExpressionType type);

    static ExpressionType reverseComparisonDirection(ExpressionType type);

    static std::string toString(ExpressionType type);
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>

namespace kuzu {
namespace common {

enum class PathSemantic : uint8_t {
    WALK = 0,
    TRAIL = 1,
    ACYCLIC = 2,
};

struct PathSemanticUtils {
    static PathSemantic fromString(const std::string& str);
    static std::string toString(PathSemantic semantic);
};

} // namespace common
} // namespace kuzu

#include <cstdint>

namespace kuzu {
namespace common {

enum class StatementType : uint8_t {
    QUERY = 0,
    CREATE_TABLE = 1,
    DROP = 2,
    ALTER = 3,
    COPY_TO = 19,
    COPY_FROM = 20,
    STANDALONE_CALL = 21,
    EXPLAIN = 22,
    CREATE_MACRO = 23,
    TRANSACTION = 30,
    EXTENSION = 31,
    EXPORT_DATABASE = 32,
    IMPORT_DATABASE = 33,
    ATTACH_DATABASE = 34,
    DETACH_DATABASE = 35,
    USE_DATABASE = 36,
    CREATE_SEQUENCE = 37,
    CREATE_TYPE = 39,
};

} // namespace common
} // namespace kuzu

#include <cstdint>

namespace kuzu {

namespace testing {
class BaseGraphTest;
class PrivateGraphTest;
class TestHelper;
class TestRunner;
class TinySnbDDLTest;
class TinySnbCopyCSVTransactionTest;
} // namespace testing

namespace benchmark {
class Benchmark;
} // namespace benchmark

namespace binder {
class Expression;
class BoundStatementResult;
class PropertyExpression;
} // namespace binder

namespace catalog {
class Catalog;
} // namespace catalog

namespace common {
enum class StatementType : uint8_t;
class Value;
struct FileInfo;
class VirtualFileSystem;
} // namespace common

namespace storage {
class MemoryManager;
class BufferManager;
class StorageManager;
class WAL;
enum class WALReplayMode : uint8_t;
} // namespace storage

namespace planner {
class LogicalPlan;
} // namespace planner

namespace processor {
class QueryProcessor;
class FactorizedTable;
class FlatTupleIterator;
class PhysicalOperator;
class PhysicalPlan;
} // namespace processor

namespace transaction {
class Transaction;
class TransactionManager;
class TransactionContext;
} // namespace transaction

} // namespace kuzu

// The Arrow C data interface.
// https://arrow.apache.org/docs/format/CDataInterface.html

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARROW_C_DATA_INTERFACE
#define ARROW_C_DATA_INTERFACE

#define ARROW_FLAG_DICTIONARY_ORDERED 1
#define ARROW_FLAG_NULLABLE 2
#define ARROW_FLAG_MAP_KEYS_SORTED 4

struct ArrowSchema {
    // Array type description
    const char* format;
    const char* name;
    const char* metadata;
    int64_t flags;
    int64_t n_children;
    struct ArrowSchema** children;
    struct ArrowSchema* dictionary;

    // Release callback
    void (*release)(struct ArrowSchema*);
    // Opaque producer-specific data
    void* private_data;
};

struct ArrowArray {
    // Array data description
    int64_t length;
    int64_t null_count;
    int64_t offset;
    int64_t n_buffers;
    int64_t n_children;
    const void** buffers;
    struct ArrowArray** children;
    struct ArrowArray* dictionary;

    // Release callback
    void (*release)(struct ArrowArray*);
    // Opaque producer-specific data
    void* private_data;
};

#endif // ARROW_C_DATA_INTERFACE

#ifdef __cplusplus
}
#endif

struct ArrowSchemaWrapper : public ArrowSchema {
    ArrowSchemaWrapper() { release = nullptr; }
    ~ArrowSchemaWrapper() {
        if (release) {
            release(this);
        }
    }
};

struct ArrowArrayWrapper : public ArrowArray {
    ArrowArrayWrapper() { release = nullptr; }
    ~ArrowArrayWrapper() {
        if (release) {
            release(this);
        }
    }
};

#include <cstdint>
#include <string>

namespace kuzu {
namespace common {

// TODO(Guodong/Ziyi/Xiyang): Should we remove this and instead use `CatalogEntryType`?
enum class TableType : uint8_t {
    UNKNOWN = 0,
    NODE = 1,
    REL = 2,
    RDF = 3,
    REL_GROUP = 4,
    FOREIGN = 5,
};

struct TableTypeUtils {
    static std::string toString(TableType tableType);
};

} // namespace common
} // namespace kuzu

#include <cstdint>

namespace kuzu {
namespace common {

enum class AlterType : uint8_t {
    RENAME_TABLE = 0,

    ADD_PROPERTY = 10,
    DROP_PROPERTY = 11,
    RENAME_PROPERTY = 12,
    COMMENT = 201,
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>

namespace kuzu {
namespace common {

enum class ConflictAction : uint8_t {
    ON_CONFLICT_THROW = 0,
    ON_CONFLICT_DO_NOTHING = 1,
};

struct ConflictActionUtil {
    static std::string toString(ConflictAction conflictAction);
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>

namespace kuzu {
namespace common {

enum class RelMultiplicity : uint8_t { MANY, ONE };
struct RelMultiplicityUtils {
    static RelMultiplicity getFwd(const std::string& multiplicityStr);
    static RelMultiplicity getBwd(const std::string& multiplicityStr);
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>

namespace kuzu {
namespace catalog {

enum class CatalogEntryType : uint8_t {
    // Table entries
    NODE_TABLE_ENTRY = 0,
    REL_TABLE_ENTRY = 1,
    REL_GROUP_ENTRY = 2,
    RDF_GRAPH_ENTRY = 3,
    FOREIGN_TABLE_ENTRY = 4,
    // Macro entries
    SCALAR_MACRO_ENTRY = 10,
    // Function entries
    AGGREGATE_FUNCTION_ENTRY = 20,
    SCALAR_FUNCTION_ENTRY = 21,
    REWRITE_FUNCTION_ENTRY = 22,
    TABLE_FUNCTION_ENTRY = 23,
    GDS_FUNCTION_ENTRY = 24,
    COPY_FUNCTION_ENTRY = 25,
    // Sequence entries
    SEQUENCE_ENTRY = 40,
    // UDT entries
    TYPE_ENTRY = 41,
    // Dummy entry
    DUMMY_ENTRY = 100,
};

struct CatalogEntryTypeUtils {
    static std::string toString(CatalogEntryType type);
};

struct FunctionEntryTypeUtils {
    static std::string toString(CatalogEntryType type);
};

} // namespace catalog
} // namespace kuzu

#include <cstdint>

namespace kuzu {
namespace common {

class Writer {
public:
    virtual void write(const uint8_t* data, uint64_t size) = 0;
    virtual ~Writer() = default;
};

} // namespace common
} // namespace kuzu

#include <cstdint>

namespace kuzu {
namespace common {

enum class ZoneMapCheckResult : uint8_t {
    ALWAYS_SCAN = 0,
    SKIP_SCAN = 1,
};

}
} // namespace kuzu

#include <mutex>

namespace kuzu {
namespace common {

struct UniqLock {
    UniqLock() {}
    explicit UniqLock(std::mutex& mtx) : lck{mtx} {}

    UniqLock(const UniqLock&) = delete;
    UniqLock& operator=(const UniqLock&) = delete;

    UniqLock(UniqLock&& other) noexcept { std::swap(lck, other.lck); }
    UniqLock& operator=(UniqLock&& other) noexcept {
        std::swap(lck, other.lck);
        return *this;
    }
    bool isLocked() const { return lck.owns_lock(); }

private:
    std::unique_lock<std::mutex> lck;
};

} // namespace common
} // namespace kuzu

#include <cstdint>

namespace kuzu {
namespace common {

class Reader {
public:
    virtual void read(uint8_t* data, uint64_t size) = 0;
    virtual ~Reader() = default;

    virtual bool finished() = 0;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>


namespace kuzu {
namespace common {

struct timestamp_t;
struct date_t;

enum class KUZU_API DatePartSpecifier : uint8_t {
    YEAR,
    MONTH,
    DAY,
    DECADE,
    CENTURY,
    MILLENNIUM,
    QUARTER,
    MICROSECOND,
    MILLISECOND,
    SECOND,
    MINUTE,
    HOUR,
    WEEK,
};

struct KUZU_API interval_t {
    int32_t months = 0;
    int32_t days = 0;
    int64_t micros = 0;

    interval_t();
    interval_t(int32_t months_p, int32_t days_p, int64_t micros_p);

    // comparator operators
    bool operator==(const interval_t& rhs) const;
    bool operator!=(const interval_t& rhs) const;

    bool operator>(const interval_t& rhs) const;
    bool operator<=(const interval_t& rhs) const;
    bool operator<(const interval_t& rhs) const;
    bool operator>=(const interval_t& rhs) const;

    // arithmetic operators
    interval_t operator+(const interval_t& rhs) const;
    timestamp_t operator+(const timestamp_t& rhs) const;
    date_t operator+(const date_t& rhs) const;
    interval_t operator-(const interval_t& rhs) const;

    interval_t operator/(const uint64_t& rhs) const;
};

// Note: Aside from some minor changes, this implementation is copied from DuckDB's source code:
// https://github.com/duckdb/duckdb/blob/master/src/include/duckdb/common/types/interval.hpp.
// https://github.com/duckdb/duckdb/blob/master/src/common/types/interval.cpp.
// When more functionality is needed, we should first consult these DuckDB links.
// The Interval class is a static class that holds helper functions for the Interval type.
class Interval {
public:
    static constexpr const int32_t MONTHS_PER_MILLENIUM = 12000;
    static constexpr const int32_t MONTHS_PER_CENTURY = 1200;
    static constexpr const int32_t MONTHS_PER_DECADE = 120;
    static constexpr const int32_t MONTHS_PER_YEAR = 12;
    static constexpr const int32_t MONTHS_PER_QUARTER = 3;
    static constexpr const int32_t DAYS_PER_WEEK = 7;
    //! only used for interval comparison/ordering purposes, in which case a month counts as 30 days
    static constexpr const int64_t DAYS_PER_MONTH = 30;
    static constexpr const int64_t DAYS_PER_YEAR = 365;
    static constexpr const int64_t MSECS_PER_SEC = 1000;
    static constexpr const int32_t SECS_PER_MINUTE = 60;
    static constexpr const int32_t MINS_PER_HOUR = 60;
    static constexpr const int32_t HOURS_PER_DAY = 24;
    static constexpr const int32_t SECS_PER_HOUR = SECS_PER_MINUTE * MINS_PER_HOUR;
    static constexpr const int32_t SECS_PER_DAY = SECS_PER_HOUR * HOURS_PER_DAY;
    static constexpr const int32_t SECS_PER_WEEK = SECS_PER_DAY * DAYS_PER_WEEK;

    static constexpr const int64_t MICROS_PER_MSEC = 1000;
    static constexpr const int64_t MICROS_PER_SEC = MICROS_PER_MSEC * MSECS_PER_SEC;
    static constexpr const int64_t MICROS_PER_MINUTE = MICROS_PER_SEC * SECS_PER_MINUTE;
    static constexpr const int64_t MICROS_PER_HOUR = MICROS_PER_MINUTE * MINS_PER_HOUR;
    static constexpr const int64_t MICROS_PER_DAY = MICROS_PER_HOUR * HOURS_PER_DAY;
    static constexpr const int64_t MICROS_PER_WEEK = MICROS_PER_DAY * DAYS_PER_WEEK;
    static constexpr const int64_t MICROS_PER_MONTH = MICROS_PER_DAY * DAYS_PER_MONTH;

    static constexpr const int64_t NANOS_PER_MICRO = 1000;
    static constexpr const int64_t NANOS_PER_MSEC = NANOS_PER_MICRO * MICROS_PER_MSEC;
    static constexpr const int64_t NANOS_PER_SEC = NANOS_PER_MSEC * MSECS_PER_SEC;
    static constexpr const int64_t NANOS_PER_MINUTE = NANOS_PER_SEC * SECS_PER_MINUTE;
    static constexpr const int64_t NANOS_PER_HOUR = NANOS_PER_MINUTE * MINS_PER_HOUR;
    static constexpr const int64_t NANOS_PER_DAY = NANOS_PER_HOUR * HOURS_PER_DAY;
    static constexpr const int64_t NANOS_PER_WEEK = NANOS_PER_DAY * DAYS_PER_WEEK;

    KUZU_API static void addition(interval_t& result, uint64_t number, std::string specifierStr);
    KUZU_API static interval_t fromCString(const char* str, uint64_t len);
    KUZU_API static std::string toString(interval_t interval);
    KUZU_API static bool greaterThan(const interval_t& left, const interval_t& right);
    KUZU_API static void normalizeIntervalEntries(interval_t input, int64_t& months, int64_t& days,
        int64_t& micros);
    KUZU_API static void tryGetDatePartSpecifier(std::string specifier, DatePartSpecifier& result);
    KUZU_API static int32_t getIntervalPart(DatePartSpecifier specifier, interval_t& timestamp);
    KUZU_API static int64_t getMicro(const interval_t& val);
    KUZU_API static int64_t getNanoseconds(const interval_t& val);
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>


namespace kuzu {
namespace common {

// Type used to represent time (microseconds)
struct KUZU_API dtime_t {
    int64_t micros;

    dtime_t();
    explicit dtime_t(int64_t micros_p);
    dtime_t& operator=(int64_t micros_p);

    // explicit conversion
    explicit operator int64_t() const;
    explicit operator double() const;

    // comparison operators
    bool operator==(const dtime_t& rhs) const;
    bool operator!=(const dtime_t& rhs) const;
    bool operator<=(const dtime_t& rhs) const;
    bool operator<(const dtime_t& rhs) const;
    bool operator>(const dtime_t& rhs) const;
    bool operator>=(const dtime_t& rhs) const;
};

// Note: Aside from some minor changes, this implementation is copied from DuckDB's source code:
// https://github.com/duckdb/duckdb/blob/master/src/include/duckdb/common/types/time.hpp.
// https://github.com/duckdb/duckdb/blob/master/src/common/types/time.cpp.
// For example, instead of using their idx_t type to refer to indices, we directly use uint64_t,
// which is the actual type of idx_t (so we say uint64_t len instead of idx_t len). When more
// functionality is needed, we should first consult these DuckDB links.
class Time {
public:
    // Convert a string in the format "hh:mm:ss" to a time object
    KUZU_API static dtime_t fromCString(const char* buf, uint64_t len);
    KUZU_API static bool tryConvertInterval(const char* buf, uint64_t len, uint64_t& pos,
        dtime_t& result);
    KUZU_API static bool tryConvertTime(const char* buf, uint64_t len, uint64_t& pos,
        dtime_t& result);

    // Convert a time object to a string in the format "hh:mm:ss"
    KUZU_API static std::string toString(dtime_t time);

    KUZU_API static dtime_t fromTime(int32_t hour, int32_t minute, int32_t second,
        int32_t microseconds = 0);

    // Extract the time from a given timestamp object
    KUZU_API static void convert(dtime_t time, int32_t& out_hour, int32_t& out_min,
        int32_t& out_sec, int32_t& out_micros);

    KUZU_API static bool isValid(int32_t hour, int32_t minute, int32_t second,
        int32_t milliseconds);

private:
    static bool tryConvertInternal(const char* buf, uint64_t len, uint64_t& pos, dtime_t& result);
    static dtime_t fromTimeInternal(int32_t hour, int32_t minute, int32_t second,
        int32_t microseconds = 0);
};

} // namespace common
} // namespace kuzu
// =========================================================================================
// This int128 implementtaion got

// =========================================================================================

#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>


namespace kuzu {
namespace common {

struct KUZU_API int128_t;

// System representation for int128_t.
struct int128_t {
    uint64_t low;
    int64_t high;

    int128_t() = default;
    int128_t(int64_t value);  // NOLINT: Allow implicit conversion from numeric values
    int128_t(int32_t value);  // NOLINT: Allow implicit conversion from numeric values
    int128_t(int16_t value);  // NOLINT: Allow implicit conversion from numeric values
    int128_t(int8_t value);   // NOLINT: Allow implicit conversion from numeric values
    int128_t(uint64_t value); // NOLINT: Allow implicit conversion from numeric values
    int128_t(uint32_t value); // NOLINT: Allow implicit conversion from numeric values
    int128_t(uint16_t value); // NOLINT: Allow implicit conversion from numeric values
    int128_t(uint8_t value);  // NOLINT: Allow implicit conversion from numeric values
    int128_t(double value);   // NOLINT: Allow implicit conversion from numeric values
    int128_t(float value);    // NOLINT: Allow implicit conversion from numeric values

    constexpr int128_t(uint64_t low, int64_t high) : low(low), high(high) {}

    constexpr int128_t(const int128_t&) = default;
    constexpr int128_t(int128_t&&) = default;
    int128_t& operator=(const int128_t&) = default;
    int128_t& operator=(int128_t&&) = default;

    int128_t operator-() const;

    // inplace arithmetic operators
    int128_t& operator+=(const int128_t& rhs);
    int128_t& operator*=(const int128_t& rhs);
    int128_t& operator|=(const int128_t& rhs);
    int128_t& operator&=(const int128_t& rhs);

    // cast operators
    explicit operator int64_t() const;
    explicit operator int32_t() const;
    explicit operator int16_t() const;
    explicit operator int8_t() const;
    explicit operator uint64_t() const;
    explicit operator uint32_t() const;
    explicit operator uint16_t() const;
    explicit operator uint8_t() const;
    explicit operator double() const;
    explicit operator float() const;
};

// arithmetic operators
KUZU_API int128_t operator+(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator-(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator*(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator/(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator%(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator^(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator&(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator~(const int128_t& val);
KUZU_API int128_t operator|(const int128_t& lhs, const int128_t& rhs);
KUZU_API int128_t operator<<(const int128_t& lhs, int amount);
KUZU_API int128_t operator>>(const int128_t& lhs, int amount);

// comparison operators
KUZU_API bool operator==(const int128_t& lhs, const int128_t& rhs);
KUZU_API bool operator!=(const int128_t& lhs, const int128_t& rhs);
KUZU_API bool operator>(const int128_t& lhs, const int128_t& rhs);
KUZU_API bool operator>=(const int128_t& lhs, const int128_t& rhs);
KUZU_API bool operator<(const int128_t& lhs, const int128_t& rhs);
KUZU_API bool operator<=(const int128_t& lhs, const int128_t& rhs);

class Int128_t {
public:
    static std::string ToString(int128_t input);

    template<class T>
    static bool tryCast(int128_t input, T& result);

    template<class T>
    static T Cast(int128_t input) {
        T result;
        tryCast(input, result);
        return result;
    }

    template<class T>
    static bool tryCastTo(T value, int128_t& result);

    template<class T>
    static int128_t castTo(T value) {
        int128_t result;
        if (!tryCastTo(value, result)) {
            throw std::overflow_error("INT128 is out of range");
        }
        return result;
    }

    // negate
    static void negateInPlace(int128_t& input) {
        if (input.high == INT64_MIN && input.low == 0) {
            throw std::overflow_error("INT128 is out of range: cannot negate INT128_MIN");
        }
        input.low = UINT64_MAX + 1 - input.low;
        input.high = -input.high - 1 + (input.low == 0);
    }

    static int128_t negate(int128_t input) {
        negateInPlace(input);
        return input;
    }

    static bool tryMultiply(int128_t lhs, int128_t rhs, int128_t& result);

    static int128_t Add(int128_t lhs, int128_t rhs);
    static int128_t Sub(int128_t lhs, int128_t rhs);
    static int128_t Mul(int128_t lhs, int128_t rhs);
    static int128_t Div(int128_t lhs, int128_t rhs);
    static int128_t Mod(int128_t lhs, int128_t rhs);
    static int128_t Xor(int128_t lhs, int128_t rhs);
    static int128_t LeftShift(int128_t lhs, int amount);
    static int128_t RightShift(int128_t lhs, int amount);
    static int128_t BinaryAnd(int128_t lhs, int128_t rhs);
    static int128_t BinaryOr(int128_t lhs, int128_t rhs);
    static int128_t BinaryNot(int128_t val);

    static int128_t divMod(int128_t lhs, int128_t rhs, int128_t& remainder);
    static int128_t divModPositive(int128_t lhs, uint64_t rhs, uint64_t& remainder);

    static bool addInPlace(int128_t& lhs, int128_t rhs);
    static bool subInPlace(int128_t& lhs, int128_t rhs);

    // comparison operators
    static bool Equals(int128_t lhs, int128_t rhs) {
        return lhs.low == rhs.low && lhs.high == rhs.high;
    }

    static bool notEquals(int128_t lhs, int128_t rhs) {
        return lhs.low != rhs.low || lhs.high != rhs.high;
    }

    static bool greaterThan(int128_t lhs, int128_t rhs) {
        return (lhs.high > rhs.high) || (lhs.high == rhs.high && lhs.low > rhs.low);
    }

    static bool greaterThanOrEquals(int128_t lhs, int128_t rhs) {
        return (lhs.high > rhs.high) || (lhs.high == rhs.high && lhs.low >= rhs.low);
    }

    static bool lessThan(int128_t lhs, int128_t rhs) {
        return (lhs.high < rhs.high) || (lhs.high == rhs.high && lhs.low < rhs.low);
    }

    static bool lessThanOrEquals(int128_t lhs, int128_t rhs) {
        return (lhs.high < rhs.high) || (lhs.high == rhs.high && lhs.low <= rhs.low);
    }
    static const int128_t powerOf10[40];
};

template<>
bool Int128_t::tryCast(int128_t input, int8_t& result);
template<>
bool Int128_t::tryCast(int128_t input, int16_t& result);
template<>
bool Int128_t::tryCast(int128_t input, int32_t& result);
template<>
bool Int128_t::tryCast(int128_t input, int64_t& result);
template<>
bool Int128_t::tryCast(int128_t input, uint8_t& result);
template<>
bool Int128_t::tryCast(int128_t input, uint16_t& result);
template<>
bool Int128_t::tryCast(int128_t input, uint32_t& result);
template<>
bool Int128_t::tryCast(int128_t input, uint64_t& result);
template<>
bool Int128_t::tryCast(int128_t input, float& result);
template<>
bool Int128_t::tryCast(int128_t input, double& result);
template<>
bool Int128_t::tryCast(int128_t input, long double& result);

template<>
bool Int128_t::tryCastTo(int8_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(int16_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(int32_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(int64_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(uint8_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(uint16_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(uint32_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(uint64_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(int128_t value, int128_t& result);
template<>
bool Int128_t::tryCastTo(float value, int128_t& result);
template<>
bool Int128_t::tryCastTo(double value, int128_t& result);
template<>
bool Int128_t::tryCastTo(long double value, int128_t& result);

// TODO: const char to int128

} // namespace common
} // namespace kuzu

template<>
struct std::hash<kuzu::common::int128_t> {
    std::size_t operator()(const kuzu::common::int128_t& v) const noexcept;
};

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace kuzu {
namespace common {

// table id type alias
using table_id_t = uint64_t;
using table_id_vector_t = std::vector<table_id_t>;
using table_id_set_t = std::unordered_set<table_id_t>;
template<typename T>
using table_id_map_t = std::unordered_map<table_id_t, T>;
constexpr table_id_t INVALID_TABLE_ID = UINT64_MAX;

// offset type alias
using offset_t = uint64_t;
constexpr offset_t INVALID_OFFSET = UINT64_MAX;

// internal id type alias
struct internalID_t;
using nodeID_t = internalID_t;
using relID_t = internalID_t;

// System representation for internalID.
struct KUZU_API internalID_t {
    offset_t offset;
    table_id_t tableID;

    internalID_t();
    internalID_t(offset_t offset, table_id_t tableID);

    // comparison operators
    bool operator==(const internalID_t& rhs) const;
    bool operator!=(const internalID_t& rhs) const;
    bool operator>(const internalID_t& rhs) const;
    bool operator>=(const internalID_t& rhs) const;
    bool operator<(const internalID_t& rhs) const;
    bool operator<=(const internalID_t& rhs) const;
};

} // namespace common
} // namespace kuzu

#include <exception>
#include <string>


namespace kuzu {
namespace common {

class KUZU_API Exception : public std::exception {
public:
    explicit Exception(std::string msg);

public:
    const char* what() const noexcept override { return exception_message_.c_str(); }

private:
    std::string exception_message_;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>


namespace kuzu {
namespace common {

class Value;

/**
 * @brief NodeVal represents a node in the graph and stores the nodeID, label and properties of that
 * node.
 */
class NodeVal {
public:
    /**
     * @return all properties of the NodeVal.
     * @note this function copies all the properties into a vector, which is not efficient. use
     * `getPropertyName` and `getPropertyVal` instead if possible.
     */
    KUZU_API static std::vector<std::pair<std::string, std::unique_ptr<Value>>> getProperties(
        const Value* val);
    /**
     * @return number of properties of the RelVal.
     */
    KUZU_API static uint64_t getNumProperties(const Value* val);

    /**
     * @return the name of the property at the given index.
     */
    KUZU_API static std::string getPropertyName(const Value* val, uint64_t index);

    /**
     * @return the value of the property at the given index.
     */
    KUZU_API static Value* getPropertyVal(const Value* val, uint64_t index);
    /**
     * @return the nodeID as a Value.
     */
    KUZU_API static Value* getNodeIDVal(const Value* val);
    /**
     * @return the name of the node as a Value.
     */
    KUZU_API static Value* getLabelVal(const Value* val);
    /**
     * @return the current node values in string format.
     */
    KUZU_API static std::string toString(const Value* val);

private:
    static void throwIfNotNode(const Value* val);
    // 2 offsets for id and label.
    static constexpr uint64_t OFFSET = 2;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <cstring>
#include <string>


namespace kuzu {
namespace common {

struct KUZU_API ku_string_t {

    static constexpr uint64_t PREFIX_LENGTH = 4;
    static constexpr uint64_t INLINED_SUFFIX_LENGTH = 8;
    static constexpr uint64_t SHORT_STR_LENGTH = PREFIX_LENGTH + INLINED_SUFFIX_LENGTH;

    uint32_t len;
    uint8_t prefix[PREFIX_LENGTH];
    union {
        uint8_t data[INLINED_SUFFIX_LENGTH];
        uint64_t overflowPtr;
    };

    ku_string_t() : len{0}, overflowPtr{0} {}
    ku_string_t(const char* value, uint64_t length);

    static bool isShortString(uint32_t len) { return len <= SHORT_STR_LENGTH; }

    const uint8_t* getData() const {
        return isShortString(len) ? prefix : reinterpret_cast<uint8_t*>(overflowPtr);
    }

    uint8_t* getDataUnsafe() {
        return isShortString(len) ? prefix : reinterpret_cast<uint8_t*>(overflowPtr);
    }

    // These functions do *NOT* allocate/resize the overflow buffer, it only copies the content and
    // set the length.
    void set(const std::string& value);
    void set(const char* value, uint64_t length);
    void set(const ku_string_t& value);
    void setShortString(const char* value, uint64_t length) {
        this->len = length;
        memcpy(prefix, value, length);
    }
    void setLongString(const char* value, uint64_t length) {
        this->len = length;
        memcpy(prefix, value, PREFIX_LENGTH);
        memcpy(reinterpret_cast<char*>(overflowPtr), value, length);
    }
    void setShortString(const ku_string_t& value) {
        this->len = value.len;
        memcpy(prefix, value.prefix, value.len);
    }
    void setLongString(const ku_string_t& value) {
        this->len = value.len;
        memcpy(prefix, value.prefix, PREFIX_LENGTH);
        memcpy(reinterpret_cast<char*>(overflowPtr), reinterpret_cast<char*>(value.overflowPtr),
            value.len);
    }

    void setFromRawStr(const char* value, uint64_t length) {
        this->len = length;
        if (isShortString(length)) {
            setShortString(value, length);
        } else {
            memcpy(prefix, value, PREFIX_LENGTH);
            overflowPtr = reinterpret_cast<uint64_t>(value);
        }
    }

    std::string getAsShortString() const;
    std::string getAsString() const;
    std::string_view getAsStringView() const;

    bool operator==(const ku_string_t& rhs) const;

    inline bool operator!=(const ku_string_t& rhs) const { return !(*this == rhs); }

    bool operator>(const ku_string_t& rhs) const;

    inline bool operator>=(const ku_string_t& rhs) const { return (*this > rhs) || (*this == rhs); }

    inline bool operator<(const ku_string_t& rhs) const { return !(*this >= rhs); }

    inline bool operator<=(const ku_string_t& rhs) const { return !(*this > rhs); }
};

} // namespace common
} // namespace kuzu

#include <cstdint>


namespace kuzu {
namespace common {

class Value;

class NestedVal {
public:
    KUZU_API static uint32_t getChildrenSize(const Value* val);

    KUZU_API static Value* getChildVal(const Value* val, uint32_t idx);
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

class Value;

/**
 * @brief RecursiveRelVal represents a path in the graph and stores the corresponding rels and nodes
 * of that path.
 */
class RecursiveRelVal {
public:
    /**
     * @return the list of nodes in the recursive rel as a Value.
     */
    KUZU_API static Value* getNodes(const Value* val);

    /**
     * @return the list of rels in the recursive rel as a Value.
     */
    KUZU_API static Value* getRels(const Value* val);

private:
    static void throwIfNotRecursiveRel(const Value* val);
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>


namespace kuzu {
namespace common {

class Value;

/**
 * @brief RelVal represents a rel in the graph and stores the relID, src/dst nodes and properties of
 * that rel.
 */
class RelVal {
public:
    /**
     * @return all properties of the RelVal.
     * @note this function copies all the properties into a vector, which is not efficient. use
     * `getPropertyName` and `getPropertyVal` instead if possible.
     */
    KUZU_API static std::vector<std::pair<std::string, std::unique_ptr<Value>>> getProperties(
        const Value* val);
    /**
     * @return number of properties of the RelVal.
     */
    KUZU_API static uint64_t getNumProperties(const Value* val);
    /**
     * @return the name of the property at the given index.
     */
    KUZU_API static std::string getPropertyName(const Value* val, uint64_t index);
    /**
     * @return the value of the property at the given index.
     */
    KUZU_API static Value* getPropertyVal(const Value* val, uint64_t index);
    /**
     * @return the src nodeID value of the RelVal in Value.
     */
    KUZU_API static Value* getSrcNodeIDVal(const Value* val);
    /**
     * @return the dst nodeID value of the RelVal in Value.
     */
    KUZU_API static Value* getDstNodeIDVal(const Value* val);
    /**
     * @return the internal ID value of the RelVal in Value.
     */
    KUZU_API static Value* getIDVal(const Value* val);
    /**
     * @return the label value of the RelVal.
     */
    KUZU_API static Value* getLabelVal(const Value* val);
    /**
     * @return the value of the RelVal in string format.
     */
    KUZU_API static std::string toString(const Value* val);

private:
    static void throwIfNotRel(const Value* val);
    // 4 offset for id, label, src, dst.
    static constexpr uint64_t OFFSET = 4;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>
#include <unordered_map>


namespace kuzu {
namespace common {

struct CaseInsensitiveStringHashFunction {
    KUZU_API uint64_t operator()(const std::string& str) const;
};

struct CaseInsensitiveStringEquality {
    KUZU_API bool operator()(const std::string& lhs, const std::string& rhs) const;
};

template<typename T>
using case_insensitive_map_t = std::unordered_map<std::string, T, CaseInsensitiveStringHashFunction,
    CaseInsensitiveStringEquality>;

} // namespace common
} // namespace kuzu
#include <cstdint>

namespace kuzu {
namespace main {

struct Version {
public:
    /**
     * @brief Get the version of the Kùzu library.
     * @return const char* The version of the Kùzu library.
     */
    KUZU_API static const char* getVersion();

    /**
     * @brief Get the storage version of the Kùzu library.
     * @return uint64_t The storage version of the Kùzu library.
     */
    KUZU_API static uint64_t getStorageVersion();
};
} // namespace main
} // namespace kuzu

#include <cstdint>
#include <string>
#include <unordered_map>


namespace kuzu {
namespace storage {

using storage_version_t = uint64_t;

struct StorageVersionInfo {
    static std::unordered_map<std::string, storage_version_t> getStorageVersionInfo() {
        return {{"0.6.0", 28}, {"0.5.0", 28}, {"0.4.2", 27}, {"0.4.1", 27}, {"0.4.0", 27},
            {"0.3.2", 26}, {"0.3.1", 26}, {"0.3.0", 26}, {"0.2.1", 25}, {"0.2.0", 25},
            {"0.1.0", 24}, {"0.0.12.3", 24}, {"0.0.12.2", 24}, {"0.0.12.1", 24}, {"0.0.12", 23},
            {"0.0.11", 23}, {"0.0.10", 23}, {"0.0.9", 23}, {"0.0.8", 17}, {"0.0.7", 15},
            {"0.0.6", 9}, {"0.0.5", 8}, {"0.0.4", 7}, {"0.0.3", 1}};
    }

    static KUZU_API storage_version_t getStorageVersion();

    static constexpr const char* MAGIC_BYTES = "KUZU";
};

} // namespace storage
} // namespace kuzu

#include <cstdint>
#include <string>


namespace kuzu {
namespace main {

struct ClientConfig {
    // System home directory.
    std::string homeDirectory;
    // File search path.
    std::string fileSearchPath;
    // If using semi mask in join.
    bool enableSemiMask;
    // If using zone map in scan.
    bool enableZoneMap;
    // Number of threads for execution.
    uint64_t numThreads;
    // Timeout (milliseconds).
    uint64_t timeoutInMS;
    // Variable length maximum depth.
    uint32_t varLengthMaxDepth;
    // If using progress bar.
    bool enableProgressBar;
    // time before displaying progress bar
    uint64_t showProgressAfter;
    // Semantic for recursive pattern, can be either WALK, TRAIL, ACYCLIC
    common::PathSemantic recursivePatternSemantic;
    // Scale factor for recursive pattern cardinality estimation.
    uint32_t recursivePatternCardinalityScaleFactor;
    bool disableMapKeyCheck;
};

struct ClientConfigDefault {
    // 0 means timeout is disabled by default.
    static constexpr uint64_t TIMEOUT_IN_MS = 0;
    static constexpr uint32_t VAR_LENGTH_MAX_DEPTH = 30;
    static constexpr bool ENABLE_SEMI_MASK = true;
    static constexpr bool ENABLE_ZONE_MAP = false;
    static constexpr bool ENABLE_PROGRESS_BAR = false;
    static constexpr uint64_t SHOW_PROGRESS_AFTER = 1000;
    static constexpr common::PathSemantic RECURSIVE_PATTERN_SEMANTIC = common::PathSemantic::WALK;
    static constexpr uint32_t RECURSIVE_PATTERN_FACTOR = 1;
    static constexpr bool DISABLE_MAP_KEY_CHECK = true;
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace main {

/**
 * @brief PreparedSummary stores the compiling time and query options of a query.
 */
struct PreparedSummary {
    double compilingTime = 0;
    common::StatementType statementType;
};

/**
 * @brief QuerySummary stores the execution time, plan, compiling time and query options of a query.
 */
class QuerySummary {
    friend class ClientContext;
    friend class benchmark::Benchmark;

public:
    /**
     * @return query compiling time in milliseconds.
     */
    KUZU_API double getCompilingTime() const;
    /**
     * @return query execution time in milliseconds.
     */
    KUZU_API double getExecutionTime() const;

    void setPreparedSummary(PreparedSummary preparedSummary_);

    /**
     * @return true if the query is executed with EXPLAIN.
     */
    bool isExplain() const;

private:
    double executionTime = 0;
    PreparedSummary preparedSummary;
};

} // namespace main
} // namespace kuzu

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace kuzu {
namespace common {

class Serializer {
public:
    explicit Serializer(std::shared_ptr<Writer> writer) : writer(std::move(writer)) {}

    Writer& getWriter() const { return *writer; }

    template<typename T>
        requires std::is_trivially_destructible<T>::value || std::is_same<std::string, T>::value
    void serializeValue(const T& value) {
        writer->write((uint8_t*)&value, sizeof(T));
    }

    // Alias for serializeValue
    template<typename T>
    void write(const T& value) {
        serializeValue(value);
    }

    void writeDebuggingInfo(const std::string& value);

    void write(const uint8_t* value, uint64_t len) { writer->write(value, len); }

    template<typename T>
    void serializeOptionalValue(const std::unique_ptr<T>& value) {
        serializeValue(value == nullptr);
        if (value != nullptr) {
            value->serialize(*this);
        }
    }

    template<typename T1, typename T2>
    void serializeUnorderedMap(const std::unordered_map<T1, std::unique_ptr<T2>>& values) {
        uint64_t mapSize = values.size();
        serializeValue(mapSize);
        for (auto& value : values) {
            serializeValue(value.first);
            value.second->serialize(*this);
        }
    }

    template<typename T>
    void serializeVector(const std::vector<T>& values) {
        uint64_t vectorSize = values.size();
        serializeValue<uint64_t>(vectorSize);
        for (auto& value : values) {
            if constexpr (requires(Serializer& ser) { value.serialize(ser); }) {
                value.serialize(*this);
            } else {
                serializeValue<T>(value);
            }
        }
    }

    template<typename T, uint64_t ARRAY_SIZE>
    void serializeArray(const std::array<T, ARRAY_SIZE>& values) {
        for (auto& value : values) {
            if constexpr (requires(Serializer& ser) { value.serialize(ser); }) {
                value.serialize(*this);
            } else {
                serializeValue<T>(value);
            }
        }
    }

    template<typename T>
    void serializeVectorOfPtrs(const std::vector<std::unique_ptr<T>>& values) {
        uint64_t vectorSize = values.size();
        serializeValue<uint64_t>(vectorSize);
        for (auto& value : values) {
            value->serialize(*this);
        }
    }

    template<typename T>
    void serializeUnorderedSet(const std::unordered_set<T>& values) {
        uint64_t setSize = values.size();
        serializeValue(setSize);
        for (const auto& value : values) {
            serializeValue(value);
        }
    }

private:
    std::shared_ptr<Writer> writer;
};

template<>
void Serializer::serializeValue(const std::string& value);

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

struct timestamp_t;

// System representation of dates as the number of days since 1970-01-01.
struct KUZU_API date_t {
    int32_t days;

    date_t();
    explicit date_t(int32_t days_p);

    // Comparison operators with date_t.
    bool operator==(const date_t& rhs) const;
    bool operator!=(const date_t& rhs) const;
    bool operator<=(const date_t& rhs) const;
    bool operator<(const date_t& rhs) const;
    bool operator>(const date_t& rhs) const;
    bool operator>=(const date_t& rhs) const;

    // Comparison operators with timestamp_t.
    bool operator==(const timestamp_t& rhs) const;
    bool operator!=(const timestamp_t& rhs) const;
    bool operator<(const timestamp_t& rhs) const;
    bool operator<=(const timestamp_t& rhs) const;
    bool operator>(const timestamp_t& rhs) const;
    bool operator>=(const timestamp_t& rhs) const;

    // arithmetic operators
    date_t operator+(const int32_t& day) const;
    date_t operator-(const int32_t& day) const;

    date_t operator+(const interval_t& interval) const;
    date_t operator-(const interval_t& interval) const;

    int64_t operator-(const date_t& rhs) const;
};

inline date_t operator+(int64_t i, const date_t date) {
    return date + i;
}

// Note: Aside from some minor changes, this implementation is copied from DuckDB's source code:
// https://github.com/duckdb/duckdb/blob/master/src/include/duckdb/common/types/date.hpp.
// https://github.com/duckdb/duckdb/blob/master/src/common/types/date.cpp.
// For example, instead of using their idx_t type to refer to indices, we directly use uint64_t,
// which is the actual type of idx_t (so we say uint64_t len instead of idx_t len). When more
// functionality is needed, we should first consult these DuckDB links.
class Date {
public:
    KUZU_API static const int32_t NORMAL_DAYS[13];
    KUZU_API static const int32_t CUMULATIVE_DAYS[13];
    KUZU_API static const int32_t LEAP_DAYS[13];
    KUZU_API static const int32_t CUMULATIVE_LEAP_DAYS[13];
    KUZU_API static const int32_t CUMULATIVE_YEAR_DAYS[401];
    KUZU_API static const int8_t MONTH_PER_DAY_OF_YEAR[365];
    KUZU_API static const int8_t LEAP_MONTH_PER_DAY_OF_YEAR[366];

    KUZU_API constexpr static const int32_t MIN_YEAR = -290307;
    KUZU_API constexpr static const int32_t MAX_YEAR = 294247;
    KUZU_API constexpr static const int32_t EPOCH_YEAR = 1970;

    KUZU_API constexpr static const int32_t YEAR_INTERVAL = 400;
    KUZU_API constexpr static const int32_t DAYS_PER_YEAR_INTERVAL = 146097;
    constexpr static const char* BC_SUFFIX = " (BC)";

    // Convert a string in the format "YYYY-MM-DD" to a date object
    KUZU_API static date_t fromCString(const char* str, uint64_t len);
    // Convert a date object to a string in the format "YYYY-MM-DD"
    KUZU_API static std::string toString(date_t date);
    // Try to convert text in a buffer to a date; returns true if parsing was successful
    KUZU_API static bool tryConvertDate(const char* buf, uint64_t len, uint64_t& pos,
        date_t& result);

    // private:
    // Returns true if (year) is a leap year, and false otherwise
    KUZU_API static bool isLeapYear(int32_t year);
    // Returns true if the specified (year, month, day) combination is a valid
    // date
    KUZU_API static bool isValid(int32_t year, int32_t month, int32_t day);
    // Extract the year, month and day from a given date object
    KUZU_API static void convert(date_t date, int32_t& out_year, int32_t& out_month,
        int32_t& out_day);
    // Create a Date object from a specified (year, month, day) combination
    KUZU_API static date_t fromDate(int32_t year, int32_t month, int32_t day);

    // Helper function to parse two digits from a string (e.g. "30" -> 30, "03" -> 3, "3" -> 3)
    KUZU_API static bool parseDoubleDigit(const char* buf, uint64_t len, uint64_t& pos,
        int32_t& result);

    KUZU_API static int32_t monthDays(int32_t year, int32_t month);

    KUZU_API static std::string getDayName(date_t& date);

    KUZU_API static std::string getMonthName(date_t& date);

    KUZU_API static date_t getLastDay(date_t& date);

    KUZU_API static int32_t getDatePart(DatePartSpecifier specifier, date_t& date);

    KUZU_API static date_t trunc(DatePartSpecifier specifier, date_t& date);

    KUZU_API static int64_t getEpochNanoSeconds(const date_t& date);

private:
    static void extractYearOffset(int32_t& n, int32_t& year, int32_t& year_offset);
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

class RandomEngine;

// Note: uuid_t is a reserved keyword in MSVC, we have to use ku_uuid_t instead.
struct ku_uuid_t {
    int128_t value;
};

struct UUID {
    static constexpr const uint8_t UUID_STRING_LENGTH = 36;
    static constexpr const char HEX_DIGITS[] = "0123456789abcdef";
    static void byteToHex(char byteVal, char* buf, uint64_t& pos);
    static unsigned char hex2Char(char ch);
    static bool isHex(char ch);
    static bool fromString(std::string str, int128_t& result);

    static int128_t fromString(std::string str);
    static int128_t fromCString(const char* str, uint64_t len);
    static void toString(int128_t input, char* buf);
    static std::string toString(int128_t input);
    static std::string toString(ku_uuid_t val);

    static ku_uuid_t generateRandomUUID(RandomEngine* engine);
};

} // namespace common
} // namespace kuzu

#include <memory>
#include <mutex>


namespace kuzu {
namespace common {

// Note: Classes in this file are NOT thread-safe.
struct MaskUtil {
    static common::offset_t getVectorIdx(common::offset_t offset) {
        return offset >> common::DEFAULT_VECTOR_CAPACITY_LOG_2;
    }
};

struct MaskData {
    uint8_t* data;

    explicit MaskData(uint64_t size, uint8_t defaultVal = 0) : size{size} {
        dataBuffer = std::make_unique<uint8_t[]>(size);
        data = dataBuffer.get();
        std::fill(data, data + size, defaultVal);
    }

    inline void setMask(uint64_t pos, uint8_t maskValue) const { data[pos] = maskValue; }
    inline bool isMasked(uint64_t pos, uint8_t trueMaskVal) const {
        return data[pos] == trueMaskVal;
    }
    inline uint8_t getMaskValue(uint64_t pos) const { return data[pos]; }
    inline uint64_t getSize() const { return size; }

private:
    std::unique_ptr<uint8_t[]> dataBuffer;
    uint64_t size;
};

// MaskCollection represents multiple mask on the same domain with AND semantic.
class MaskCollection {
public:
    MaskCollection() : numMasks{0} {}

    void init(common::offset_t maxOffset) {
        std::unique_lock lck{mtx};
        if (maskData != nullptr) {
            // MaskCollection might be initialized repeatedly. Because multiple semiMasker can
            // hold the same mask.
            return;
        }
        maskData = std::make_unique<MaskData>(maxOffset + 1);
    }

    // Return true if any offset between [startOffset, endOffset] is masked. Otherwise return false.
    bool isMasked(common::offset_t startOffset, common::offset_t endOffset) const {
        auto offset = startOffset;
        auto numMasked = 0u;
        while (offset <= endOffset) {
            numMasked += maskData->isMasked(offset++, numMasks);
        }
        return numMasked > 0;
    }
    // Increment mask value for the given nodeOffset if its current mask value is equal to
    // the specified `currentMaskValue`.
    // Note: blindly update mask does not parallelize well, so we minimize write by first checking
    // if the mask is set to true (mask value is equal to the expected currentMaskValue) or not.
    void incrementMaskValue(common::offset_t offset, uint8_t currentMaskValue) {
        if (offset >= maskData->getSize()) [[unlikely]] { // Handle uncommitted node offsets.
            return;
        }
        if (maskData->isMasked(offset, currentMaskValue)) {
            maskData->setMask(offset, currentMaskValue + 1);
        }
    }

    uint8_t getNumMasks() const { return numMasks; }
    void incrementNumMasks() { numMasks++; }

private:
    std::mutex mtx;
    std::unique_ptr<MaskData> maskData;
    uint8_t numMasks;
};

class NodeSemiMask {
public:
    explicit NodeSemiMask(common::table_id_t tableID, common::offset_t maxOffset)
        : tableID{tableID}, maxOffset{maxOffset} {}
    virtual ~NodeSemiMask() = default;

    common::table_id_t getTableID() const { return tableID; }
    common::offset_t getMaxOffset() const { return maxOffset; }

    virtual void init() = 0;

    virtual void incrementMaskValue(common::offset_t nodeOffset, uint8_t currentMaskValue) = 0;
    virtual bool isMasked(common::offset_t startNodeOffset, common::offset_t endNodeOffset) = 0;

    bool isEnabled() const { return getNumMasks() > 0; }
    uint8_t getNumMasks() const { return maskCollection.getNumMasks(); }
    void incrementNumMasks() { maskCollection.incrementNumMasks(); }

protected:
    common::table_id_t tableID;
    common::offset_t maxOffset;
    MaskCollection maskCollection;
};

class NodeOffsetLevelSemiMask final : public NodeSemiMask {
public:
    explicit NodeOffsetLevelSemiMask(common::table_id_t tableID, common::offset_t maxOffset)
        : NodeSemiMask{tableID, maxOffset} {}

    void init() override {
        if (maxOffset == common::INVALID_OFFSET) {
            return;
        }
        maskCollection.init(maxOffset + 1);
    }

    void incrementMaskValue(common::offset_t nodeOffset, uint8_t currentMaskValue) override {
        maskCollection.incrementMaskValue(nodeOffset, currentMaskValue);
    }

    bool isMasked(common::offset_t startNodeOffset, common::offset_t endNodeOffset) override {
        return maskCollection.isMasked(startNodeOffset, endNodeOffset);
    }
};

class NodeVectorLevelSemiMask final : public NodeSemiMask {
public:
    explicit NodeVectorLevelSemiMask(common::table_id_t tableID, common::offset_t maxOffset)
        : NodeSemiMask{tableID, maxOffset} {}

    void init() override {
        if (maxOffset == common::INVALID_OFFSET) {
            return;
        }
        maskCollection.init(MaskUtil::getVectorIdx(maxOffset) + 1);
    }

    void incrementMaskValue(uint64_t nodeOffset, uint8_t currentMaskValue) override {
        maskCollection.incrementMaskValue(MaskUtil::getVectorIdx(nodeOffset), currentMaskValue);
    }

    bool isMasked(common::offset_t startNodeOffset, common::offset_t endNodeOffset) override {
        return maskCollection.isMasked(MaskUtil::getVectorIdx(startNodeOffset),
            MaskUtil::getVectorIdx(endNodeOffset));
    }
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace storage {

enum class DBFileType : uint8_t {
    NODE_INDEX = 0,
    DATA = 1,
};

// DBFileID start with 1 byte type  followed with additional bytes needed by node hash index
// (isOverflow and tableID).
struct DBFileID {
    DBFileType dbFileType;
    bool isOverflow = false;
    common::table_id_t tableID = common::INVALID_TABLE_ID;

    DBFileID() = default;
    bool operator==(const DBFileID& rhs) const = default;

    static DBFileID newDataFileID();
    static DBFileID newPKIndexFileID(common::table_id_t tableID);
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace common {

class KUZU_API InternalException : public Exception {
public:
    explicit InternalException(const std::string& msg) : Exception(msg){};
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

class KUZU_API BinderException : public Exception {
public:
    explicit BinderException(const std::string& msg) : Exception("Binder exception: " + msg){};
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

class KUZU_API CatalogException : public Exception {
public:
    explicit CatalogException(const std::string& msg) : Exception("Catalog exception: " + msg){};
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

struct blob_t {
    ku_string_t value;
};

struct HexFormatConstants {
    // map of integer -> hex value.
    static constexpr const char* HEX_TABLE = "0123456789ABCDEF";
    // reverse map of byte -> integer value, or -1 for invalid hex values.
    static const int HEX_MAP[256];
    static constexpr const uint64_t NUM_BYTES_TO_SHIFT_FOR_FIRST_BYTE = 4;
    static constexpr const uint64_t SECOND_BYTE_MASK = 0x0F;
    static constexpr const char PREFIX[] = "\\x";
    static constexpr const uint64_t PREFIX_LENGTH = 2;
    static constexpr const uint64_t FIRST_BYTE_POS = PREFIX_LENGTH;
    static constexpr const uint64_t SECOND_BYTES_POS = PREFIX_LENGTH + 1;
    static constexpr const uint64_t LENGTH = 4;
};

struct Blob {
    static std::string toString(const uint8_t* value, uint64_t len);

    static inline std::string toString(const blob_t& blob) {
        return toString(blob.value.getData(), blob.value.len);
    }

    static uint64_t getBlobSize(const ku_string_t& blob);

    static uint64_t fromString(const char* str, uint64_t length, uint8_t* resultBuffer);

    template<typename T>
    static inline T getValue(const blob_t& data) {
        return *reinterpret_cast<const T*>(data.value.getData());
    }
    template<typename T>
    // NOLINTNEXTLINE(readability-non-const-parameter): Would cast away qualifiers.
    static inline T getValue(char* data) {
        return *reinterpret_cast<T*>(data);
    }

private:
    static void validateHexCode(const uint8_t* blobStr, uint64_t length, uint64_t curPos);
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

// Type used to represent timestamps (value is in microseconds since 1970-01-01)
struct KUZU_API timestamp_t {
    int64_t value = 0;

    timestamp_t();
    explicit timestamp_t(int64_t value_p);
    timestamp_t& operator=(int64_t value_p);

    // explicit conversion
    explicit operator int64_t() const;

    // Comparison operators with timestamp_t.
    bool operator==(const timestamp_t& rhs) const;
    bool operator!=(const timestamp_t& rhs) const;
    bool operator<=(const timestamp_t& rhs) const;
    bool operator<(const timestamp_t& rhs) const;
    bool operator>(const timestamp_t& rhs) const;
    bool operator>=(const timestamp_t& rhs) const;

    // Comparison operators with date_t.
    bool operator==(const date_t& rhs) const;
    bool operator!=(const date_t& rhs) const;
    bool operator<(const date_t& rhs) const;
    bool operator<=(const date_t& rhs) const;
    bool operator>(const date_t& rhs) const;
    bool operator>=(const date_t& rhs) const;

    // arithmetic operator
    timestamp_t operator+(const interval_t& interval) const;
    timestamp_t operator-(const interval_t& interval) const;

    interval_t operator-(const timestamp_t& rhs) const;
};

struct timestamp_tz_t : public timestamp_t { // NO LINT
    using timestamp_t::timestamp_t;
};
struct timestamp_ns_t : public timestamp_t { // NO LINT
    using timestamp_t::timestamp_t;
};
struct timestamp_ms_t : public timestamp_t { // NO LINT
    using timestamp_t::timestamp_t;
};
struct timestamp_sec_t : public timestamp_t { // NO LINT
    using timestamp_t::timestamp_t;
};

// Note: Aside from some minor changes, this implementation is copied from DuckDB's source code:
// https://github.com/duckdb/duckdb/blob/master/src/include/duckdb/common/types/timestamp.hpp.
// https://github.com/duckdb/duckdb/blob/master/src/common/types/timestamp.cpp.
// For example, instead of using their idx_t type to refer to indices, we directly use uint64_t,
// which is the actual type of idx_t (so we say uint64_t len instead of idx_t len). When more
// functionality is needed, we should first consult these DuckDB links.

// The Timestamp class is a static class that holds helper functions for the Timestamp type.
// timestamp/datetime uses 64 bits, high 32 bits for date and low 32 bits for time
class Timestamp {
public:
    KUZU_API static timestamp_t fromCString(const char* str, uint64_t len);

    // Convert a timestamp object to a std::string in the format "YYYY-MM-DD hh:mm:ss".
    KUZU_API static std::string toString(timestamp_t timestamp);

    KUZU_API static date_t getDate(timestamp_t timestamp);

    KUZU_API static dtime_t getTime(timestamp_t timestamp);

    // Create a Timestamp object from a specified (date, time) combination.
    KUZU_API static timestamp_t fromDateTime(date_t date, dtime_t time);

    KUZU_API static bool tryConvertTimestamp(const char* str, uint64_t len, timestamp_t& result);

    // Extract the date and time from a given timestamp object.
    KUZU_API static void convert(timestamp_t timestamp, date_t& out_date, dtime_t& out_time);

    // Create a Timestamp object from the specified epochMs.
    KUZU_API static timestamp_t fromEpochMicroSeconds(int64_t epochMs);

    // Create a Timestamp object from the specified epochMs.
    KUZU_API static timestamp_t fromEpochMilliSeconds(int64_t ms);

    // Create a Timestamp object from the specified epochSec.
    KUZU_API static timestamp_t fromEpochSeconds(int64_t sec);

    // Create a Timestamp object from the specified epochNs.
    KUZU_API static timestamp_t fromEpochNanoSeconds(int64_t ns);

    KUZU_API static int32_t getTimestampPart(DatePartSpecifier specifier, timestamp_t& timestamp);

    KUZU_API static timestamp_t trunc(DatePartSpecifier specifier, timestamp_t& date);

    KUZU_API static int64_t getEpochNanoSeconds(const timestamp_t& timestamp);

    KUZU_API static int64_t getEpochMilliSeconds(const timestamp_t& timestamp);

    KUZU_API static int64_t getEpochSeconds(const timestamp_t& timestamp);

    KUZU_API static bool tryParseUTCOffset(const char* str, uint64_t& pos, uint64_t len,
        int& hour_offset, int& minute_offset);

    static std::string getTimestampConversionExceptionMsg(const char* str, uint64_t len,
        const std::string& typeID = "TIMESTAMP") {
        return "Error occurred during parsing " + typeID + ". Given: \"" + std::string(str, len) +
               "\". Expected format: (YYYY-MM-DD hh:mm:ss[.zzzzzz][+-TT[:tt]])";
    }

    KUZU_API static timestamp_t getCurrentTimestamp();
};

} // namespace common
} // namespace kuzu

#include <string>
#include <string_view>
#include <type_traits>


namespace kuzu {
namespace common {

namespace string_format_detail {
#define MAP_STD_TO_STRING(typ)                                                                     \
    inline std::string map(typ v) {                                                                \
        return std::to_string(v);                                                                  \
    }

MAP_STD_TO_STRING(short)
MAP_STD_TO_STRING(unsigned short)
MAP_STD_TO_STRING(int)
MAP_STD_TO_STRING(unsigned int)
MAP_STD_TO_STRING(long)
MAP_STD_TO_STRING(unsigned long)
MAP_STD_TO_STRING(long long)
MAP_STD_TO_STRING(unsigned long long)
MAP_STD_TO_STRING(float)
MAP_STD_TO_STRING(double)
#undef MAP_STD_TO_STRING

#define MAP_SELF(typ)                                                                              \
    inline typ map(typ v) {                                                                        \
        return v;                                                                                  \
    }
MAP_SELF(const char*);
// Also covers std::string
MAP_SELF(std::string_view)

// chars are mapped to themselves, but signed char and unsigned char (which are used for int8_t and
// uint8_t respectively), need to be cast to be properly output as integers. This is consistent with
// fmt's behaviour.
MAP_SELF(char)
inline std::string map(signed char v) {
    return std::to_string(int(v));
}
inline std::string map(unsigned char v) {
    return std::to_string(unsigned(v));
}
#undef MAP_SELF

template<typename... Args>
inline void stringFormatHelper(std::string& ret, std::string_view format, Args&&... args) {
    size_t bracket = format.find('{');
    if (bracket == std::string_view::npos) {
        ret += format;
        return;
    }
    ret += format.substr(0, bracket);
    if (format.substr(bracket, 4) == "{{}}") {
        // Escaped {}.
        ret += "{}";
        return stringFormatHelper(ret, format.substr(bracket + 4), std::forward<Args>(args)...);
    } else if (format.substr(bracket, 2) == "{}") {
        // Formatted {}.
        throw InternalException("Not enough values for string_format.");
    }
    // Something else.
    ret.push_back('{');
    return stringFormatHelper(ret, format.substr(bracket + 1), std::forward<Args>(args)...);
}

template<typename Arg, typename... Args>
inline void stringFormatHelper(std::string& ret, std::string_view format, Arg&& arg,
    Args&&... args) {
    size_t bracket = format.find('{');
    if (bracket == std::string_view::npos) {
        throw InternalException("Too many values for string_format.");
    }
    ret += format.substr(0, bracket);
    if (format.substr(bracket, 4) == "{{}}") {
        // Escaped {}.
        ret += "{}";
        return stringFormatHelper(ret, format.substr(bracket + 4), std::forward<Arg>(arg),
            std::forward<Args>(args)...);
    } else if (format.substr(bracket, 2) == "{}") {
        // Formatted {}.
        ret += map(arg);
        return stringFormatHelper(ret, format.substr(bracket + 2), std::forward<Args>(args)...);
    }
    // Something else.
    ret.push_back('{');
    return stringFormatHelper(ret, format.substr(bracket + 1), std::forward<Arg>(arg),
        std::forward<Args>(args)...);
}
} // namespace string_format_detail

//! Formats `args` according to `format`. Accepts {} for formatting the argument and {{}} for
//! a literal {}. Formatting is done with std::ostream::operator<<.
template<typename... Args>
inline std::string stringFormat(std::string_view format, Args... args) {
    std::string ret;
    ret.reserve(32); // Optimistic pre-allocation.
    string_format_detail::stringFormatHelper(ret, format, std::forward<Args>(args)...);
    return ret;
}

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

[[noreturn]] inline void kuAssertFailureInternal(const char* condition_name, const char* file,
    int linenr) {
    // LCOV_EXCL_START
    throw InternalException(stringFormat("Assertion failed in file \"{}\" on line {}: {}", file,
        linenr, condition_name));
    // LCOV_EXCL_STOP
}

#if defined(KUZU_RUNTIME_CHECKS) || !defined(NDEBUG)
#define RUNTIME_CHECK(code) code
#define KU_ASSERT(condition)                                                                       \
    static_cast<bool>(condition) ?                                                                 \
        void(0) :                                                                                  \
        kuzu::common::kuAssertFailureInternal(#condition, __FILE__, __LINE__)
#else
#define KU_ASSERT(condition) void(0)
#define RUNTIME_CHECK(code) void(0)
#endif

#define KU_UNREACHABLE                                                                             \
    [[unlikely]] kuzu::common::kuAssertFailureInternal("KU_UNREACHABLE", __FILE__, __LINE__)
#define KU_UNUSED(expr) (void)(expr)

} // namespace common
} // namespace kuzu

#include <typeinfo>


namespace kuzu {
namespace common {

template<typename FROM, typename TO>
TO ku_dynamic_cast(FROM old) {
#if defined(KUZU_RUNTIME_CHECKS) || !defined(NDEBUG)
    try {
        TO newVal = dynamic_cast<TO>(old);
        if constexpr (std::is_pointer<FROM>()) {
            KU_ASSERT(newVal != nullptr);
        }
        return newVal;
    } catch (std::bad_cast& e) {
        KU_ASSERT(false);
    }
#else
    return reinterpret_cast<TO>(old);
#endif
}

} // namespace common
} // namespace kuzu

#include <chrono>
#include <string>


namespace kuzu {
namespace common {

class Timer {

public:
    void start() {
        finished = false;
        startTime = std::chrono::high_resolution_clock::now();
    }

    void stop() {
        stopTime = std::chrono::high_resolution_clock::now();
        finished = true;
    }

    double getDuration() const {
        if (finished) {
            auto duration = stopTime - startTime;
            return (double)std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
        }
        throw Exception("Timer is still running.");
    }

    uint64_t getElapsedTimeInMS() const {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now - startTime;
        auto count = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        KU_ASSERT(count >= 0);
        return count;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> stopTime;
    bool finished = false;
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <string>


namespace kuzu {
namespace storage {

enum class ResidencyState : uint8_t { IN_MEMORY = 0, ON_DISK = 1 };

struct ResidencyStateUtils {
    static std::string toString(ResidencyState residencyState) {
        switch (residencyState) {
        case ResidencyState::IN_MEMORY: {
            return "IN_MEMORY";
        }
        case ResidencyState::ON_DISK: {
            return "ON_DISK";
        }
        default: {
            KU_UNREACHABLE;
        }
        }
    }
};

} // namespace storage
} // namespace kuzu

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace kuzu {
namespace common {

class Deserializer {
public:
    explicit Deserializer(std::unique_ptr<Reader> reader) : reader(std::move(reader)) {}

    bool finished() const { return reader->finished(); }

    template<typename T>
        requires std::is_trivially_destructible_v<T> || std::is_same_v<std::string, T>
    void deserializeValue(T& value) {
        reader->read(reinterpret_cast<uint8_t*>(&value), sizeof(T));
    }

    void read(uint8_t* data, uint64_t size) { reader->read(data, size); }

    void validateDebuggingInfo(std::string& value, std::string expectedVal);

    template<typename T>
    void deserializeOptionalValue(std::unique_ptr<T>& value) {
        bool isNull;
        deserializeValue(isNull);
        if (!isNull) {
            value = T::deserialize(*this);
        }
    }

    template<typename T1, typename T2>
    void deserializeUnorderedMap(std::unordered_map<T1, std::unique_ptr<T2>>& values) {
        uint64_t mapSize;
        deserializeValue<uint64_t>(mapSize);
        values.reserve(mapSize);
        for (auto i = 0u; i < mapSize; i++) {
            T1 key;
            deserializeValue<T1>(key);
            auto val = T2::deserialize(*this);
            values.emplace(key, std::move(val));
        }
    }

    template<typename T>
    void deserializeVector(std::vector<T>& values) {
        uint64_t vectorSize;
        deserializeValue(vectorSize);
        values.resize(vectorSize);
        for (auto& value : values) {
            if constexpr (requires(Deserializer& deser) { T::deserialize(deser); }) {
                value = T::deserialize(*this);
            } else {
                deserializeValue(value);
            }
        }
    }

    template<typename T, uint64_t ARRAY_SIZE>
    void deserializeArray(std::array<T, ARRAY_SIZE>& values) {
        KU_ASSERT(values.size() == ARRAY_SIZE);
        for (auto& value : values) {
            if constexpr (requires(Deserializer& deser) { T::deserialize(deser); }) {
                value = T::deserialize(*this);
            } else {
                deserializeValue(value);
            }
        }
    }

    template<typename T>
    void deserializeVectorOfPtrs(std::vector<std::unique_ptr<T>>& values) {
        uint64_t vectorSize;
        deserializeValue(vectorSize);
        values.resize(vectorSize);
        for (auto i = 0u; i < vectorSize; i++) {
            values[i] = T::deserialize(*this);
        }
    }

    template<typename T>
    void deserializeUnorderedSet(std::unordered_set<T>& values) {
        uint64_t setSize;
        deserializeValue(setSize);
        for (auto i = 0u; i < setSize; i++) {
            T value;
            deserializeValue<T>(value);
            values.insert(value);
        }
    }

private:
    std::unique_ptr<Reader> reader;
};

template<>
void Deserializer::deserializeValue(std::string& value);

} // namespace common
} // namespace kuzu

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace kuzu {
namespace processor {
class ParquetReader;
};
namespace common {

class Serializer;
class Deserializer;
struct FileInfo;

using sel_t = uint64_t;
using hash_t = uint64_t;
using page_idx_t = uint32_t;
using frame_idx_t = page_idx_t;
using page_offset_t = uint32_t;
constexpr page_idx_t INVALID_PAGE_IDX = UINT32_MAX;
using file_idx_t = uint32_t;
constexpr file_idx_t INVALID_FILE_IDX = UINT32_MAX;
using page_group_idx_t = uint32_t;
using frame_group_idx_t = page_group_idx_t;
using property_id_t = uint32_t;
constexpr property_id_t INVALID_PROPERTY_ID = UINT32_MAX;
using column_id_t = property_id_t;
constexpr column_id_t INVALID_COLUMN_ID = INVALID_PROPERTY_ID;
constexpr column_id_t ROW_IDX_COLUMN_ID = INVALID_COLUMN_ID - 1;
using idx_t = uint32_t;
constexpr idx_t INVALID_IDX = UINT32_MAX;
using block_idx_t = uint64_t;
constexpr block_idx_t INVALID_BLOCK_IDX = UINT64_MAX;
using struct_field_idx_t = uint8_t;
using union_field_idx_t = struct_field_idx_t;
constexpr struct_field_idx_t INVALID_STRUCT_FIELD_IDX = UINT8_MAX;
using row_idx_t = uint64_t;
constexpr row_idx_t INVALID_ROW_IDX = UINT64_MAX;
constexpr uint32_t UNDEFINED_CAST_COST = UINT32_MAX;
using node_group_idx_t = uint64_t;
constexpr node_group_idx_t INVALID_NODE_GROUP_IDX = UINT64_MAX;
using partition_idx_t = uint64_t;
constexpr partition_idx_t INVALID_PARTITION_IDX = UINT64_MAX;
using length_t = uint64_t;
using list_size_t = uint32_t;
using sequence_id_t = uint64_t;

using transaction_t = uint64_t;
constexpr transaction_t INVALID_TRANSACTION = UINT64_MAX;
using executor_id_t = uint64_t;
using executor_info = std::unordered_map<executor_id_t, uint64_t>;

// System representation for a variable-sized overflow value.
struct overflow_value_t {
    // the size of the overflow buffer can be calculated as:
    // numElements * sizeof(Element) + nullMap(4 bytes alignment)
    uint64_t numElements = 0;
    uint8_t* value = nullptr;
};

struct list_entry_t {
    offset_t offset;
    list_size_t size;

    list_entry_t() : offset{INVALID_OFFSET}, size{UINT32_MAX} {}
    list_entry_t(offset_t offset, list_size_t size) : offset{offset}, size{size} {}
};

struct struct_entry_t {
    int64_t pos;
};

struct map_entry_t {
    list_entry_t entry;
};

struct union_entry_t {
    struct_entry_t entry;
};

struct int128_t;
struct ku_string_t;

template<typename T>
concept IntegerTypes =
    std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
    std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, int128_t>;

template<typename T>
concept NumericTypes = IntegerTypes<T> || std::floating_point<T>;

template<typename T>
concept ComparableTypes = NumericTypes<T> || std::is_same_v<T, ku_string_t> ||
                          std::is_same_v<T, interval_t> || std::is_same_v<T, bool>;

template<typename T>
concept HashablePrimitive = ((std::integral<T> && !std::is_same_v<T, bool>) ||
                             std::floating_point<T> || std::is_same_v<T, int128_t>);
template<typename T>
concept IndexHashable = ((std::integral<T> && !std::is_same_v<T, bool>) || std::floating_point<T> ||
                         std::is_same_v<T, int128_t> || std::is_same_v<T, ku_string_t> ||
                         std::is_same_v<T, std::string_view> || std::same_as<T, std::string>);

template<typename T>
concept HashableNonNestedTypes = (std::integral<T> || std::floating_point<T> ||
                                  std::is_same_v<T, int128_t> || std::is_same_v<T, internalID_t> ||
                                  std::is_same_v<T, interval_t> || std::is_same_v<T, ku_string_t>);

template<typename T>
concept HashableNestedTypes =
    (std::is_same_v<T, list_entry_t> || std::is_same_v<T, struct_entry_t>);

template<typename T>
concept HashableTypes = (HashableNestedTypes<T> || HashableNonNestedTypes<T>);

enum class LogicalTypeID : uint8_t {
    ANY = 0,
    NODE = 10,
    REL = 11,
    RECURSIVE_REL = 12,
    // SERIAL is a special data type that is used to represent a sequence of INT64 values that are
    // incremented by 1 starting from 0.
    SERIAL = 13,

    BOOL = 22,
    INT64 = 23,
    INT32 = 24,
    INT16 = 25,
    INT8 = 26,
    UINT64 = 27,
    UINT32 = 28,
    UINT16 = 29,
    UINT8 = 30,
    INT128 = 31,
    DOUBLE = 32,
    FLOAT = 33,
    DATE = 34,
    TIMESTAMP = 35,
    TIMESTAMP_SEC = 36,
    TIMESTAMP_MS = 37,
    TIMESTAMP_NS = 38,
    TIMESTAMP_TZ = 39,
    INTERVAL = 40,
    DECIMAL = 41,
    INTERNAL_ID = 42,

    STRING = 50,
    BLOB = 51,

    LIST = 52,
    ARRAY = 53,
    STRUCT = 54,
    MAP = 55,
    UNION = 56,
    RDF_VARIANT = 57,
    POINTER = 58,

    UUID = 59,
};

enum class PhysicalTypeID : uint8_t {
    // Fixed size types.
    ANY = 0,
    BOOL = 1,
    INT64 = 2,
    INT32 = 3,
    INT16 = 4,
    INT8 = 5,
    UINT64 = 6,
    UINT32 = 7,
    UINT16 = 8,
    UINT8 = 9,
    INT128 = 10,
    DOUBLE = 11,
    FLOAT = 12,
    INTERVAL = 13,
    INTERNAL_ID = 14,

    // Variable size types.
    STRING = 20,
    LIST = 22,
    ARRAY = 23,
    STRUCT = 24,
    POINTER = 25,
};

class ExtraTypeInfo;
class StructField;
class StructTypeInfo;

class LogicalType {
    friend struct LogicalTypeUtils;
    friend struct DecimalType;
    friend struct StructType;
    friend struct ListType;
    friend struct ArrayType;

    KUZU_API LogicalType(const LogicalType& other);

public:
    KUZU_API LogicalType() : typeID{LogicalTypeID::ANY}, extraTypeInfo{nullptr} {
        physicalType = getPhysicalType(this->typeID);
    };
    explicit KUZU_API LogicalType(LogicalTypeID typeID);
    EXPLICIT_COPY_DEFAULT_MOVE(LogicalType);

    KUZU_API bool operator==(const LogicalType& other) const;
    KUZU_API bool operator!=(const LogicalType& other) const;

    KUZU_API std::string toString() const;
    static bool tryConvertFromString(const std::string& str, LogicalType& type);
    static LogicalType fromString(const std::string& str);

    KUZU_API LogicalTypeID getLogicalTypeID() const { return typeID; }
    bool containsAny() const;

    KUZU_API PhysicalTypeID getPhysicalType() const { return physicalType; }
    KUZU_API static PhysicalTypeID getPhysicalType(LogicalTypeID logicalType,
        const std::unique_ptr<ExtraTypeInfo>& extraTypeInfo = nullptr);

    void setExtraTypeInfo(std::unique_ptr<ExtraTypeInfo> typeInfo) {
        extraTypeInfo = std::move(typeInfo);
    }

    void serialize(Serializer& serializer) const;

    static LogicalType deserialize(Deserializer& deserializer);

    KUZU_API static std::vector<LogicalType> copy(const std::vector<LogicalType>& types);
    KUZU_API static std::vector<LogicalType> copy(const std::vector<LogicalType*>& types);

    static LogicalType ANY() { return LogicalType(LogicalTypeID::ANY); }
    static LogicalType BOOL() { return LogicalType(LogicalTypeID::BOOL); }
    static LogicalType HASH() { return LogicalType(LogicalTypeID::UINT64); }
    static LogicalType INT64() { return LogicalType(LogicalTypeID::INT64); }
    static LogicalType INT32() { return LogicalType(LogicalTypeID::INT32); }
    static LogicalType INT16() { return LogicalType(LogicalTypeID::INT16); }
    static LogicalType INT8() { return LogicalType(LogicalTypeID::INT8); }
    static LogicalType UINT64() { return LogicalType(LogicalTypeID::UINT64); }
    static LogicalType UINT32() { return LogicalType(LogicalTypeID::UINT32); }
    static LogicalType UINT16() { return LogicalType(LogicalTypeID::UINT16); }
    static LogicalType UINT8() { return LogicalType(LogicalTypeID::UINT8); }
    static LogicalType INT128() { return LogicalType(LogicalTypeID::INT128); }
    static LogicalType DOUBLE() { return LogicalType(LogicalTypeID::DOUBLE); }
    static LogicalType FLOAT() { return LogicalType(LogicalTypeID::FLOAT); }
    static LogicalType DATE() { return LogicalType(LogicalTypeID::DATE); }
    static LogicalType TIMESTAMP_NS() { return LogicalType(LogicalTypeID::TIMESTAMP_NS); }
    static LogicalType TIMESTAMP_MS() { return LogicalType(LogicalTypeID::TIMESTAMP_MS); }
    static LogicalType TIMESTAMP_SEC() { return LogicalType(LogicalTypeID::TIMESTAMP_SEC); }
    static LogicalType TIMESTAMP_TZ() { return LogicalType(LogicalTypeID::TIMESTAMP_TZ); }
    static LogicalType TIMESTAMP() { return LogicalType(LogicalTypeID::TIMESTAMP); }
    static LogicalType INTERVAL() { return LogicalType(LogicalTypeID::INTERVAL); }
    static KUZU_API LogicalType DECIMAL(uint32_t precision, uint32_t scale);
    static LogicalType INTERNAL_ID() { return LogicalType(LogicalTypeID::INTERNAL_ID); }
    static LogicalType SERIAL() { return LogicalType(LogicalTypeID::SERIAL); }
    static LogicalType STRING() { return LogicalType(LogicalTypeID::STRING); }
    static LogicalType BLOB() { return LogicalType(LogicalTypeID::BLOB); }
    static LogicalType UUID() { return LogicalType(LogicalTypeID::UUID); }
    static LogicalType POINTER() { return LogicalType(LogicalTypeID::POINTER); }
    static KUZU_API LogicalType STRUCT(std::vector<StructField>&& fields);

    static KUZU_API LogicalType RECURSIVE_REL(std::unique_ptr<StructTypeInfo> typeInfo);

    static KUZU_API LogicalType NODE(std::unique_ptr<StructTypeInfo> typeInfo);

    static KUZU_API LogicalType REL(std::unique_ptr<StructTypeInfo> typeInfo);

    static KUZU_API LogicalType RDF_VARIANT();

    static KUZU_API LogicalType UNION(std::vector<StructField>&& fields);

    static KUZU_API LogicalType LIST(LogicalType childType);
    template<class T>
    static inline LogicalType LIST(T&& childType) {
        return LogicalType::LIST(LogicalType(std::forward<T>(childType)));
    }

    static KUZU_API LogicalType MAP(LogicalType keyType, LogicalType valueType);
    template<class T>
    static LogicalType MAP(T&& keyType, T&& valueType) {
        return LogicalType::MAP(LogicalType(std::forward<T>(keyType)),
            LogicalType(std::forward<T>(valueType)));
    }

    static KUZU_API LogicalType ARRAY(LogicalType childType, uint64_t numElements);
    template<class T>
    static LogicalType ARRAY(T&& childType, uint64_t numElements) {
        return LogicalType::ARRAY(LogicalType(std::forward<T>(childType)), numElements);
    }

private:
    friend struct CAPIHelper;
    friend struct JavaAPIHelper;
    friend class kuzu::processor::ParquetReader;
    explicit LogicalType(LogicalTypeID typeID, std::unique_ptr<ExtraTypeInfo> extraTypeInfo);

private:
    LogicalTypeID typeID;
    PhysicalTypeID physicalType;
    std::unique_ptr<ExtraTypeInfo> extraTypeInfo;
};

class ExtraTypeInfo {
public:
    virtual ~ExtraTypeInfo() = default;

    void serialize(Serializer& serializer) const { serializeInternal(serializer); }

    virtual bool containsAny() const = 0;

    virtual bool operator==(const ExtraTypeInfo& other) const = 0;

    virtual std::unique_ptr<ExtraTypeInfo> copy() const = 0;

    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const ExtraTypeInfo*, const TARGET*>(this);
    }

protected:
    virtual void serializeInternal(Serializer& serializer) const = 0;
};

class DecimalTypeInfo final : public ExtraTypeInfo {
public:
    explicit DecimalTypeInfo(uint32_t precision = 18, uint32_t scale = 3)
        : precision(precision), scale(scale) {}

    uint32_t getPrecision() const { return precision; }
    uint32_t getScale() const { return scale; }

    bool containsAny() const override { return false; }

    bool operator==(const ExtraTypeInfo& other) const override;

    std::unique_ptr<ExtraTypeInfo> copy() const override;

    static std::unique_ptr<ExtraTypeInfo> deserialize(Deserializer& deserializer);

protected:
    void serializeInternal(Serializer& serializer) const override;

    uint32_t precision, scale;
};

class ListTypeInfo : public ExtraTypeInfo {
public:
    ListTypeInfo() = default;
    explicit ListTypeInfo(LogicalType childType) : childType{std::move(childType)} {}

    const LogicalType& getChildType() const { return childType; }

    bool containsAny() const override;

    bool operator==(const ExtraTypeInfo& other) const override;

    std::unique_ptr<ExtraTypeInfo> copy() const override;

    static std::unique_ptr<ExtraTypeInfo> deserialize(Deserializer& deserializer);

protected:
    void serializeInternal(Serializer& serializer) const override;

protected:
    LogicalType childType;
};

class ArrayTypeInfo final : public ListTypeInfo {
public:
    ArrayTypeInfo() = default;
    explicit ArrayTypeInfo(LogicalType childType, uint64_t numElements)
        : ListTypeInfo{std::move(childType)}, numElements{numElements} {}

    uint64_t getNumElements() const { return numElements; }

    bool operator==(const ExtraTypeInfo& other) const override;

    static std::unique_ptr<ExtraTypeInfo> deserialize(Deserializer& deserializer);

    std::unique_ptr<ExtraTypeInfo> copy() const override;

private:
    void serializeInternal(Serializer& serializer) const override;

private:
    uint64_t numElements;
};

class KUZU_API StructField {
public:
    StructField() : type{LogicalType()} {}
    StructField(std::string name, LogicalType type)
        : name{std::move(name)}, type{std::move(type)} {};

    std::string getName() const { return name; }

    const LogicalType& getType() const { return type; }

    bool containsAny() const;

    bool operator==(const StructField& other) const;
    bool operator!=(const StructField& other) const { return !(*this == other); }

    void serialize(Serializer& serializer) const;

    static StructField deserialize(Deserializer& deserializer);

    StructField copy() const;

private:
    std::string name;
    LogicalType type;
};

class StructTypeInfo final : public ExtraTypeInfo {
public:
    StructTypeInfo() = default;
    explicit StructTypeInfo(std::vector<StructField>&& fields);
    StructTypeInfo(const std::vector<std::string>& fieldNames,
        const std::vector<LogicalType>& fieldTypes);

    bool hasField(const std::string& fieldName) const;
    struct_field_idx_t getStructFieldIdx(std::string fieldName) const;
    const StructField& getStructField(struct_field_idx_t idx) const;
    const StructField& getStructField(const std::string& fieldName) const;
    const std::vector<StructField>& getStructFields() const;

    const LogicalType& getChildType(struct_field_idx_t idx) const;
    std::vector<const LogicalType*> getChildrenTypes() const;
    // can't be a vector of refs since that can't be for-each looped through
    std::vector<std::string> getChildrenNames() const;

    bool containsAny() const override;

    bool operator==(const ExtraTypeInfo& other) const override;

    static std::unique_ptr<ExtraTypeInfo> deserialize(Deserializer& deserializer);
    std::unique_ptr<ExtraTypeInfo> copy() const override;

private:
    void serializeInternal(Serializer& serializer) const override;

private:
    std::vector<StructField> fields;
    std::unordered_map<std::string, struct_field_idx_t> fieldNameToIdxMap;
};

using logical_type_vec_t = std::vector<LogicalType>;

struct KUZU_API DecimalType {
    static uint32_t getPrecision(const LogicalType& type);
    static uint32_t getScale(const LogicalType& type);
    static std::string insertDecimalPoint(const std::string& value, uint32_t posFromEnd);
};

struct KUZU_API ListType {
    static const LogicalType& getChildType(const LogicalType& type);
};

struct KUZU_API ArrayType {
    static const LogicalType& getChildType(const LogicalType& type);
    static uint64_t getNumElements(const LogicalType& type);
};

struct KUZU_API StructType {
    static std::vector<const LogicalType*> getFieldTypes(const LogicalType& type);
    // since the field types isn't stored as a vector of LogicalTypes, we can't return vector<>&

    static const LogicalType& getFieldType(const LogicalType& type, struct_field_idx_t idx);

    static const LogicalType& getFieldType(const LogicalType& type, const std::string& key);

    static std::vector<std::string> getFieldNames(const LogicalType& type);

    static uint64_t getNumFields(const LogicalType& type);

    static const std::vector<StructField>& getFields(const LogicalType& type);

    static bool hasField(const LogicalType& type, const std::string& key);

    static const StructField& getField(const LogicalType& type, struct_field_idx_t idx);

    static const StructField& getField(const LogicalType& type, const std::string& key);

    static struct_field_idx_t getFieldIdx(const LogicalType& type, const std::string& key);
};

struct KUZU_API MapType {
    static const LogicalType& getKeyType(const LogicalType& type);

    static const LogicalType& getValueType(const LogicalType& type);
};

struct KUZU_API UnionType {
    static constexpr union_field_idx_t TAG_FIELD_IDX = 0;

    static constexpr auto TAG_FIELD_TYPE = LogicalTypeID::INT8;

    static constexpr char TAG_FIELD_NAME[] = "tag";

    static union_field_idx_t getInternalFieldIdx(union_field_idx_t idx);

    static std::string getFieldName(const LogicalType& type, union_field_idx_t idx);

    static const LogicalType& getFieldType(const LogicalType& type, union_field_idx_t idx);

    static uint64_t getNumFields(const LogicalType& type);
};

struct PhysicalTypeUtils {
    static std::string toString(PhysicalTypeID physicalType);
    static uint32_t getFixedTypeSize(PhysicalTypeID physicalType);
};

struct KUZU_API LogicalTypeUtils {
    static std::string toString(LogicalTypeID dataTypeID);
    static std::string toString(const std::vector<LogicalType>& dataTypes);
    static std::string toString(const std::vector<LogicalTypeID>& dataTypeIDs);
    static uint32_t getRowLayoutSize(const LogicalType& logicalType);
    static bool isDate(const LogicalType& dataType);
    static bool isDate(const LogicalTypeID& dataType);
    static bool isTimestamp(const LogicalType& dataType);
    static bool isTimestamp(const LogicalTypeID& dataType);
    static bool isUnsigned(const LogicalType& dataType);
    static bool isUnsigned(const LogicalTypeID& dataType);
    static bool isIntegral(const LogicalType& dataType);
    static bool isIntegral(const LogicalTypeID& dataType);
    static bool isNumerical(const LogicalType& dataType);
    static bool isNumerical(const LogicalTypeID& dataType);
    static bool isNested(const LogicalType& dataType);
    static bool isNested(LogicalTypeID logicalTypeID);
    static std::vector<LogicalTypeID> getAllValidComparableLogicalTypes();
    static std::vector<LogicalTypeID> getNumericalLogicalTypeIDs();
    static std::vector<LogicalTypeID> getIntegerTypeIDs();
    static std::vector<LogicalTypeID> getAllValidLogicTypeIDs();
    static std::vector<LogicalType> getAllValidLogicTypes();
    static bool tryGetMaxLogicalType(const LogicalType& left, const LogicalType& right,
        LogicalType& result);
    static bool tryGetMaxLogicalType(const std::vector<LogicalType>& types, LogicalType& result);

private:
    static bool tryGetMaxLogicalTypeID(const LogicalTypeID& left, const LogicalTypeID& right,
        LogicalTypeID& result);
};

enum class FileVersionType : uint8_t { ORIGINAL = 0, WAL_VERSION = 1 };

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace parser {

class Statement {
public:
    explicit Statement(common::StatementType statementType) : statementType{statementType} {}

    virtual ~Statement() = default;

    common::StatementType getStatementType() const { return statementType; }

    bool requireTx() {
        switch (statementType) {
        case common::StatementType::TRANSACTION:
            return false;
        default:
            return true;
        }
    }

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<Statement&, TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const Statement&, const TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const Statement*, const TARGET*>(this);
    }

private:
    common::StatementType statementType;
};

} // namespace parser
} // namespace kuzu

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace kuzu {

namespace common {
struct FileInfo;
class Serializer;
class Deserializer;
} // namespace common

namespace parser {

class ParsedExpression;
class ParsedExpressionChildrenVisitor;
using parsed_expr_vector = std::vector<std::unique_ptr<ParsedExpression>>;
using parsed_expr_pair =
    std::pair<std::unique_ptr<ParsedExpression>, std::unique_ptr<ParsedExpression>>;
using s_parsed_expr_pair = std::pair<std::string, std::unique_ptr<ParsedExpression>>;

class KUZU_API ParsedExpression {
    friend class ParsedExpressionChildrenVisitor;

public:
    ParsedExpression(common::ExpressionType type, std::unique_ptr<ParsedExpression> child,
        std::string rawName);
    ParsedExpression(common::ExpressionType type, std::unique_ptr<ParsedExpression> left,
        std::unique_ptr<ParsedExpression> right, std::string rawName);
    ParsedExpression(common::ExpressionType type, std::string rawName)
        : type{type}, rawName{std::move(rawName)} {}
    explicit ParsedExpression(common::ExpressionType type) : type{type} {}

    ParsedExpression(common::ExpressionType type, std::string alias, std::string rawName,
        parsed_expr_vector children)
        : type{type}, alias{std::move(alias)}, rawName{std::move(rawName)},
          children{std::move(children)} {}
    DELETE_COPY_DEFAULT_MOVE(ParsedExpression);
    virtual ~ParsedExpression() = default;

    common::ExpressionType getExpressionType() const { return type; }

    void setAlias(std::string name) { alias = std::move(name); }

    bool hasAlias() const { return !alias.empty(); }

    std::string getAlias() const { return alias; }

    std::string getRawName() const { return rawName; }

    uint32_t getNumChildren() const { return children.size(); }
    ParsedExpression* getChild(uint32_t idx) const { return children[idx].get(); }

    std::string toString() const { return rawName; }

    virtual std::unique_ptr<ParsedExpression> copy() const {
        return std::make_unique<ParsedExpression>(type, alias, rawName, copyVector(children));
    }

    void serialize(common::Serializer& serializer) const;

    static std::unique_ptr<ParsedExpression> deserialize(common::Deserializer& deserializer);

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<ParsedExpression&, TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const ParsedExpression&, const TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const ParsedExpression*, const TARGET*>(this);
    }

private:
    virtual void serializeInternal(common::Serializer&) const {}

protected:
    common::ExpressionType type;
    std::string alias;
    std::string rawName;
    parsed_expr_vector children;
};

using options_t = std::unordered_map<std::string, std::unique_ptr<parser::ParsedExpression>>;

struct ParsedExpressionUtils {
    static std::unique_ptr<ParsedExpression> getSerialDefaultExpr(const std::string& sequenceName);
};

} // namespace parser
} // namespace kuzu


namespace kuzu {
namespace common {

/**
 * Note that metrics are not thread safe.
 */
class Metric {

public:
    explicit Metric(bool enabled) : enabled{enabled} {}

    virtual ~Metric() = default;

public:
    bool enabled;
};

class TimeMetric : public Metric {

public:
    explicit TimeMetric(bool enable);

    void start();
    void stop();

    double getElapsedTimeMS() const;

public:
    double accumulatedTime;
    bool isStarted;
    Timer timer;
};

class NumericMetric : public Metric {

public:
    explicit NumericMetric(bool enable);

    void increase(uint64_t value);

    void incrementByOne();

public:
    uint64_t accumulatedValue;
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

struct ku_list_t {

public:
    ku_list_t() : size{0}, overflowPtr{0} {}
    ku_list_t(uint64_t size, uint64_t overflowPtr) : size{size}, overflowPtr{overflowPtr} {}

    void set(const uint8_t* values, const LogicalType& dataType) const;

private:
    void set(const std::vector<uint8_t*>& parameters, LogicalTypeID childTypeId);

public:
    uint64_t size;
    uint64_t overflowPtr;
};

} // namespace common
} // namespace kuzu

#include <memory>

#include <span>

namespace kuzu {
namespace common {

class SelectionVector {
public:
    KUZU_API static const sel_t INCREMENTAL_SELECTED_POS[DEFAULT_VECTOR_CAPACITY];

    explicit SelectionVector(sel_t capacity) : selectedSize{0}, capacity{capacity} {
        selectedPositionsBuffer = std::make_unique<sel_t[]>(capacity);
        setToUnfiltered();
    }

    SelectionVector() : SelectionVector{DEFAULT_VECTOR_CAPACITY} {}

    bool isUnfiltered() const { return selectedPositions == (sel_t*)&INCREMENTAL_SELECTED_POS; }

    void setToUnfiltered() { selectedPositions = (sel_t*)&INCREMENTAL_SELECTED_POS; }
    void setToUnfiltered(sel_t size) {
        KU_ASSERT(size <= capacity);
        selectedPositions = (sel_t*)&INCREMENTAL_SELECTED_POS;
        selectedSize = size;
    }

    // Set to filtered is not very accurate. It sets selectedPositions to a mutable array.
    void setToFiltered() { selectedPositions = selectedPositionsBuffer.get(); }
    void setToFiltered(sel_t size) {
        KU_ASSERT(size <= capacity && selectedPositionsBuffer);
        selectedPositions = selectedPositionsBuffer.get();
        selectedSize = size;
    }

    std::span<sel_t> getMultableBuffer() {
        return std::span<sel_t>(selectedPositionsBuffer.get(), capacity);
    }
    std::span<sel_t> getSelectedPositions() const {
        return std::span<sel_t>(selectedPositions, selectedSize);
    }

    sel_t getSelSize() const { return selectedSize; }
    void setSelSize(sel_t size) {
        KU_ASSERT(size <= capacity);
        selectedSize = size;
    }
    void incrementSelSize(sel_t increment = 1) {
        KU_ASSERT(selectedSize < capacity);
        selectedSize += increment;
    }

    sel_t operator[](sel_t index) const {
        KU_ASSERT(index < capacity);
        return selectedPositions[index];
    }
    sel_t& operator[](sel_t index) {
        KU_ASSERT(index < capacity);
        return selectedPositions[index];
    }

private:
    sel_t selectedSize;
    sel_t capacity;
    std::unique_ptr<sel_t[]> selectedPositionsBuffer;
    sel_t* selectedPositions;
};

} // namespace common
} // namespace kuzu


namespace arrow {
class ChunkedArray;
} // namespace arrow

namespace kuzu {
namespace common {

class ValueVector;

// AuxiliaryBuffer holds data which is only used by the targeting dataType.
class AuxiliaryBuffer {
public:
    virtual ~AuxiliaryBuffer() = default;

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<AuxiliaryBuffer&, TARGET&>(*this);
    }

    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const AuxiliaryBuffer&, const TARGET&>(*this);
    }
};

class StringAuxiliaryBuffer : public AuxiliaryBuffer {
public:
    explicit StringAuxiliaryBuffer(storage::MemoryManager* memoryManager) {
        inMemOverflowBuffer = std::make_unique<InMemOverflowBuffer>(memoryManager);
    }

    InMemOverflowBuffer* getOverflowBuffer() const { return inMemOverflowBuffer.get(); }
    uint8_t* allocateOverflow(uint64_t size) { return inMemOverflowBuffer->allocateSpace(size); }
    void resetOverflowBuffer() const { inMemOverflowBuffer->resetBuffer(); }

private:
    std::unique_ptr<InMemOverflowBuffer> inMemOverflowBuffer;
};

class StructAuxiliaryBuffer : public AuxiliaryBuffer {
public:
    StructAuxiliaryBuffer(const LogicalType& type, storage::MemoryManager* memoryManager);

    void referenceChildVector(idx_t idx, std::shared_ptr<ValueVector> vectorToReference) {
        childrenVectors[idx] = std::move(vectorToReference);
    }
    const std::vector<std::shared_ptr<ValueVector>>& getFieldVectors() const {
        return childrenVectors;
    }

private:
    std::vector<std::shared_ptr<ValueVector>> childrenVectors;
};

class ArrowColumnAuxiliaryBuffer : public AuxiliaryBuffer {
    friend class ArrowColumnVector;

private:
    std::shared_ptr<arrow::ChunkedArray> column;
};

// ListVector layout:
// To store a list value in the valueVector, we could use two separate vectors.
// 1. A vector(called offset vector) for the list offsets and length(called list_entry_t): This
// vector contains the starting indices and length for each list within the data vector.
// 2. A data vector(called dataVector) to store the actual list elements: This vector holds the
// actual elements of the lists in a flat, continuous storage. Each list would be represented as a
// contiguous subsequence of elements in this vector.
class KUZU_API ListAuxiliaryBuffer : public AuxiliaryBuffer {
    friend class ListVector;

public:
    ListAuxiliaryBuffer(const LogicalType& dataVectorType, storage::MemoryManager* memoryManager);

    void setDataVector(std::shared_ptr<ValueVector> vector) { dataVector = std::move(vector); }
    ValueVector* getDataVector() const { return dataVector.get(); }
    std::shared_ptr<ValueVector> getSharedDataVector() const { return dataVector; }

    list_entry_t addList(list_size_t listSize);

    uint64_t getSize() const { return size; }

    void resetSize() { size = 0; }

    void resize(uint64_t numValues);

private:
    void resizeDataVector(ValueVector* dataVector);

    void resizeStructDataVector(ValueVector* dataVector);

private:
    uint64_t capacity;
    uint64_t size;

    std::shared_ptr<ValueVector> dataVector;
};

class AuxiliaryBufferFactory {
public:
    static std::unique_ptr<AuxiliaryBuffer> getAuxiliaryBuffer(LogicalType& type,
        storage::MemoryManager* memoryManager);
};

} // namespace common
} // namespace kuzu

#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>


namespace kuzu {
namespace binder {

class Expression;
using expression_vector = std::vector<std::shared_ptr<Expression>>;
using expression_pair = std::pair<std::shared_ptr<Expression>, std::shared_ptr<Expression>>;

struct ExpressionHasher;
struct ExpressionEquality;
using expression_set =
    std::unordered_set<std::shared_ptr<Expression>, ExpressionHasher, ExpressionEquality>;
template<typename T>
using expression_map =
    std::unordered_map<std::shared_ptr<Expression>, T, ExpressionHasher, ExpressionEquality>;

class Expression : public std::enable_shared_from_this<Expression> {
    friend class ExpressionChildrenCollector;

public:
    Expression(common::ExpressionType expressionType, common::LogicalType dataType,
        expression_vector children, std::string uniqueName)
        : expressionType{expressionType}, dataType{std::move(dataType)},
          uniqueName{std::move(uniqueName)}, children{std::move(children)} {}
    // Create binary expression.
    Expression(common::ExpressionType expressionType, common::LogicalType dataType,
        const std::shared_ptr<Expression>& left, const std::shared_ptr<Expression>& right,
        std::string uniqueName)
        : Expression{expressionType, std::move(dataType), expression_vector{left, right},
              std::move(uniqueName)} {}
    // Create unary expression.
    Expression(common::ExpressionType expressionType, common::LogicalType dataType,
        const std::shared_ptr<Expression>& child, std::string uniqueName)
        : Expression{expressionType, std::move(dataType), expression_vector{child},
              std::move(uniqueName)} {}
    // Create leaf expression
    Expression(common::ExpressionType expressionType, common::LogicalType dataType,
        std::string uniqueName)
        : Expression{expressionType, std::move(dataType), expression_vector{},
              std::move(uniqueName)} {}
    DELETE_COPY_DEFAULT_MOVE(Expression);
    virtual ~Expression() = default;

    void setAlias(const std::string& name) { alias = name; }

    void setUniqueName(const std::string& name) { uniqueName = name; }
    std::string getUniqueName() const {
        KU_ASSERT(!uniqueName.empty());
        return uniqueName;
    }

    virtual void cast(const common::LogicalType& type);
    // NOTE: Avoid using the following unsafe getter. It is meant for resolving ANY data type only.
    common::LogicalType& getDataTypeUnsafe() { return dataType; }
    const common::LogicalType& getDataType() const { return dataType; }

    bool hasAlias() const { return !alias.empty(); }
    std::string getAlias() const { return alias; }

    uint32_t getNumChildren() const { return children.size(); }
    std::shared_ptr<Expression> getChild(common::idx_t idx) const {
        KU_ASSERT(idx < children.size());
        return children[idx];
    }
    expression_vector getChildren() const { return children; }
    void setChild(common::idx_t idx, std::shared_ptr<Expression> child) {
        KU_ASSERT(idx < children.size());
        children[idx] = std::move(child);
    }

    expression_vector splitOnAND();

    bool operator==(const Expression& rhs) const { return uniqueName == rhs.uniqueName; }

    std::string toString() const { return hasAlias() ? alias : toStringInternal(); }

    virtual std::unique_ptr<Expression> copy() const {
        throw common::InternalException("Unimplemented expression copy().");
    }

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<Expression&, TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const Expression&, const TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const Expression*, const TARGET*>(this);
    }

protected:
    virtual std::string toStringInternal() const = 0;

public:
    common::ExpressionType expressionType;
    common::LogicalType dataType;

protected:
    // Name that serves as the unique identifier.
    std::string uniqueName;
    std::string alias;
    expression_vector children;
};

struct ExpressionHasher {
    std::size_t operator()(const std::shared_ptr<Expression>& expression) const {
        return std::hash<std::string>{}(expression->getUniqueName());
    }
};

struct ExpressionEquality {
    bool operator()(const std::shared_ptr<Expression>& left,
        const std::shared_ptr<Expression>& right) const {
        return left->getUniqueName() == right->getUniqueName();
    }
};

} // namespace binder
} // namespace kuzu

#include <string>


namespace kuzu {
namespace main {
class ClientContext;
} // namespace main

namespace catalog {

class KUZU_API CatalogEntry {
public:
    //===--------------------------------------------------------------------===//
    // constructor & destructor
    //===--------------------------------------------------------------------===//
    CatalogEntry() = default;
    CatalogEntry(CatalogEntryType type, std::string name)
        : type{type}, name{std::move(name)}, timestamp{common::INVALID_TRANSACTION} {}
    DELETE_COPY_DEFAULT_MOVE(CatalogEntry);
    virtual ~CatalogEntry() = default;

    //===--------------------------------------------------------------------===//
    // getter & setter
    //===--------------------------------------------------------------------===//
    CatalogEntryType getType() const { return type; }
    void rename(std::string name_) { this->name = std::move(name_); }
    std::string getName() const { return name; }
    common::transaction_t getTimestamp() const { return timestamp; }
    void setTimestamp(common::transaction_t timestamp_) { this->timestamp = timestamp_; }
    bool isDeleted() const { return deleted; }
    void setDeleted(bool deleted_) { this->deleted = deleted_; }
    bool hasParent() const { return hasParent_; }
    void setHasParent(bool hasParent) { hasParent_ = hasParent; }
    CatalogEntry* getPrev() const {
        KU_ASSERT(prev);
        return prev.get();
    }
    std::unique_ptr<CatalogEntry> movePrev() {
        if (this->prev) {
            this->prev->setNext(nullptr);
        }
        return std::move(prev);
    }
    void setPrev(std::unique_ptr<CatalogEntry> prev_) {
        this->prev = std::move(prev_);
        if (this->prev) {
            this->prev->setNext(this);
        }
    }
    CatalogEntry* getNext() const { return next; }
    void setNext(CatalogEntry* next_) { this->next = next_; }

    //===--------------------------------------------------------------------===//
    // serialization & deserialization
    //===--------------------------------------------------------------------===//
    virtual void serialize(common::Serializer& serializer) const;
    static std::unique_ptr<CatalogEntry> deserialize(common::Deserializer& deserializer);

    virtual std::string toCypher(main::ClientContext* /*clientContext*/) const { KU_UNREACHABLE; }

    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const CatalogEntry&, const TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const CatalogEntry*, const TARGET*>(this);
    }
    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<CatalogEntry*, TARGET*>(this);
    }

protected:
    virtual void copyFrom(const CatalogEntry& other);

protected:
    CatalogEntryType type;
    std::string name;
    common::transaction_t timestamp;
    bool deleted = false;
    bool hasParent_ = false;
    // Older versions.
    std::unique_ptr<CatalogEntry> prev;
    // Newer versions.
    CatalogEntry* next = nullptr;
};

} // namespace catalog
} // namespace kuzu

#include <type_traits>

#include <bit>
#include <concepts>

namespace kuzu {
namespace common {
namespace numeric_utils {

template<typename T>
concept IsIntegral = std::integral<T> || std::same_as<std::remove_cvref_t<T>, int128_t>;

template<typename T>
concept IsSigned = std::same_as<T, int128_t> || std::numeric_limits<T>::is_signed;

template<typename T>
concept IsUnSigned = std::numeric_limits<T>::is_unsigned;

template<typename T>
struct MakeSigned {
    using type = std::make_signed_t<T>;
};

template<>
struct MakeSigned<int128_t> {
    using type = int128_t;
};

template<typename T>
using MakeSignedT = typename MakeSigned<T>::type;

template<typename T>
struct MakeUnSigned {
    using type = std::make_unsigned_t<T>;
};

template<>
struct MakeUnSigned<int128_t> {
    // currently evaluates to int128_t as we don't have an uint128_t type
    using type = int128_t;
};

template<typename T>
using MakeUnSignedT = typename MakeUnSigned<T>::type;

template<typename T>
decltype(auto) makeValueSigned(T value) {
    return static_cast<MakeSignedT<T>>(value);
}

template<typename T>
decltype(auto) makeValueUnSigned(T value) {
    return static_cast<MakeUnSignedT<T>>(value);
}

template<typename T>
constexpr int bitWidth(T x) {
    return std::bit_width(x);
}

template<>
constexpr int bitWidth<int128_t>(int128_t x) {
    if (x.high != 0) {
        constexpr size_t BITS_PER_BYTE = 8;
        return sizeof(x.low) * BITS_PER_BYTE + std::bit_width(makeValueUnSigned(x.high));
    }
    return std::bit_width(x.low);
}
} // namespace numeric_utils
} // namespace common
} // namespace kuzu

#include <vector>


namespace kuzu {
namespace storage {

template<class T>
class GroupCollection {
public:
    GroupCollection() {}

    common::UniqLock lock() { return common::UniqLock{mtx}; }

    void loadGroups(common::Deserializer& deSer) {
        lock();
        deSer.deserializeVectorOfPtrs<T>(groups);
    }
    void serializeGroups(common::Serializer& ser) {
        lock();
        ser.serializeVectorOfPtrs<T>(groups);
    }

    void appendGroup(const common::UniqLock& lock, std::unique_ptr<T> group) {
        KU_ASSERT(group);
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        groups.push_back(std::move(group));
    }
    T* getGroup(const common::UniqLock& lock, common::idx_t groupIdx) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        if (groupIdx >= groups.size()) {
            return nullptr;
        }
        return groups[groupIdx].get();
    }
    T* getGroupNoLock(common::idx_t groupIdx) {
        if (groupIdx >= groups.size()) {
            return nullptr;
        }
        return groups[groupIdx].get();
    }
    void replaceGroup(const common::UniqLock& lock, common::idx_t groupIdx,
        std::unique_ptr<T> group) {
        KU_ASSERT(group);
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        if (groupIdx >= groups.size()) {
            groups.resize(groupIdx + 1);
        }
        groups[groupIdx] = std::move(group);
    }

    void resize(const common::UniqLock& lock, common::idx_t newSize) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        if (newSize <= groups.size()) {
            return;
        }
        groups.resize(newSize);
    }

    bool isEmpty(const common::UniqLock& lock) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        return groups.empty();
    }
    common::idx_t getNumGroups(const common::UniqLock& lock) const {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        return groups.size();
    }

    const std::vector<std::unique_ptr<T>>& getAllGroups(const common::UniqLock& lock) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        return groups;
    }
    std::unique_ptr<T> moveGroup(const common::UniqLock& lock, common::idx_t groupIdx) {
        KU_ASSERT(groupIdx < groups.size());
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        return std::move(groups[groupIdx]);
    }
    T* getFirstGroup(const common::UniqLock& lock) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        if (groups.empty()) {
            return nullptr;
        }
        return groups.front().get();
    }
    T* getFirstGroupNoLock() {
        if (groups.empty()) {
            return nullptr;
        }
        return groups.front().get();
    }
    T* getLastGroup(const common::UniqLock& lock) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        if (groups.empty()) {
            return nullptr;
        }
        return groups.back().get();
    }

    void clear(const common::UniqLock& lock) {
        KU_ASSERT(lock.isLocked());
        KU_UNUSED(lock);
        groups.clear();
    }

private:
    std::mutex mtx;
    std::vector<std::unique_ptr<T>> groups;
};

} // namespace storage
} // namespace kuzu

#include <mutex>


namespace kuzu {
namespace catalog {
class CatalogEntry;
class CatalogSet;
class SequenceCatalogEntry;
struct SequenceRollbackData;
} // namespace catalog
namespace transaction {
class Transaction;
}

namespace main {
class ClientContext;
}
namespace storage {

// TODO(Guodong): This should be reworked to use MemoryManager for memory allocaiton.
//                For now, we use malloc to get around the limitation of 256KB from MM.
class UndoMemoryBuffer {
public:
    static constexpr uint64_t UNDO_MEMORY_BUFFER_SIZE = common::BufferPoolConstants::PAGE_4KB_SIZE;

    explicit UndoMemoryBuffer(uint64_t size) : size{size} {
        data = std::make_unique<uint8_t[]>(size);
        currentPosition = 0;
    }

    uint8_t* getDataUnsafe() const { return data.get(); }
    uint8_t const* getData() const { return data.get(); }
    uint64_t getSize() const { return size; }
    uint64_t getCurrentPosition() const { return currentPosition; }
    void moveCurrentPosition(uint64_t offset) {
        KU_ASSERT(currentPosition + offset <= size);
        currentPosition += offset;
    }
    bool canFit(uint64_t size_) const { return currentPosition + size_ <= this->size; }

private:
    std::unique_ptr<uint8_t[]> data;
    uint64_t size;
    uint64_t currentPosition;
};

class UndoBuffer;
class UndoBufferIterator {
public:
    explicit UndoBufferIterator(const UndoBuffer& undoBuffer) : undoBuffer{undoBuffer} {}

    template<typename F>
    void iterate(F&& callback);
    template<typename F>
    void reverseIterate(F&& callback);

private:
    const UndoBuffer& undoBuffer;
};

class UpdateInfo;
class VersionInfo;
struct VectorUpdateInfo;
struct VectorVersionInfo;
class WAL;
// This class is not thread safe, as it is supposed to be accessed by a single thread.
class UndoBuffer {
    friend class UndoBufferIterator;

public:
    enum class UndoRecordType : uint16_t {
        CATALOG_ENTRY = 0,
        SEQUENCE_ENTRY = 1,
        UPDATE_INFO = 6,
        INSERT_INFO = 7,
        DELETE_INFO = 8,
    };

    explicit UndoBuffer(transaction::Transaction* transaction);

    void createCatalogEntry(catalog::CatalogSet& catalogSet, catalog::CatalogEntry& catalogEntry);
    void createSequenceChange(catalog::SequenceCatalogEntry& sequenceEntry,
        const catalog::SequenceRollbackData& data);
    void createVectorInsertInfo(VersionInfo* versionInfo, common::idx_t vectorIdx,
        common::row_idx_t startRowInVector, common::row_idx_t numRows);
    void createVectorDeleteInfo(VersionInfo* versionInfo, common::idx_t vectorIdx,
        common::row_idx_t startRowInVector, common::row_idx_t numRows);
    void createVectorUpdateInfo(UpdateInfo* updateInfo, common::idx_t vectorIdx,
        VectorUpdateInfo* vectorUpdateInfo);

    void commit(common::transaction_t commitTS) const;
    void rollback();

    uint64_t getMemUsage() const;

private:
    uint8_t* createUndoRecord(uint64_t size);

    void createVectorVersionInfo(UndoRecordType recordType, VersionInfo* versionInfo,
        common::idx_t vectorIdx, common::row_idx_t startRowInVector, common::row_idx_t numRows);

    void commitRecord(UndoRecordType recordType, const uint8_t* record,
        common::transaction_t commitTS) const;
    void rollbackRecord(UndoRecordType recordType, const uint8_t* record);

    void commitCatalogEntryRecord(const uint8_t* record, common::transaction_t commitTS) const;
    void rollbackCatalogEntryRecord(const uint8_t* record);

    void commitSequenceEntry(uint8_t const* entry, common::transaction_t commitTS) const;
    void rollbackSequenceEntry(uint8_t const* entry);

    void commitVectorVersionInfo(UndoRecordType recordType, const uint8_t* record,
        common::transaction_t commitTS) const;
    void rollbackVectorVersionInfo(UndoRecordType recordType, const uint8_t* record);

    void commitVectorUpdateInfo(const uint8_t* record, common::transaction_t commitTS) const;
    void rollbackVectorUpdateInfo(const uint8_t* record) const;

private:
    std::mutex mtx;
    transaction::Transaction* transaction;
    std::vector<UndoMemoryBuffer> memoryBuffers;
};

} // namespace storage
} // namespace kuzu

#include <type_traits>


namespace kuzu {
namespace common {

class ValueVector;

template<class... Funcs>
struct overload : Funcs... {
    explicit overload(Funcs... funcs) : Funcs(funcs)... {}
    using Funcs::operator()...;
};

class TypeUtils {
public:
    static std::string entryToString(const LogicalType& dataType, const uint8_t* value,
        ValueVector* vector);

    template<typename T>
    static inline std::string toString(const T& val, void* /*valueVector*/ = nullptr) {
        static_assert(std::is_same<T, int64_t>::value || std::is_same<T, int32_t>::value ||
                      std::is_same<T, int16_t>::value || std::is_same<T, int8_t>::value ||
                      std::is_same<T, uint64_t>::value || std::is_same<T, uint32_t>::value ||
                      std::is_same<T, uint16_t>::value || std::is_same<T, uint8_t>::value ||
                      std::is_same<T, double>::value || std::is_same<T, float>::value);
        return std::to_string(val);
    }
    static std::string nodeToString(const struct_entry_t& val, ValueVector* vector);
    static std::string relToString(const struct_entry_t& val, ValueVector* vector);

    static inline void encodeOverflowPtr(uint64_t& overflowPtr, page_idx_t pageIdx,
        uint32_t pageOffset) {
        memcpy(&overflowPtr, &pageIdx, 4);
        memcpy(((uint8_t*)&overflowPtr) + 4, &pageOffset, 4);
    }
    static inline void decodeOverflowPtr(uint64_t overflowPtr, page_idx_t& pageIdx,
        uint32_t& pageOffset) {
        pageIdx = 0;
        memcpy(&pageIdx, &overflowPtr, 4);
        memcpy(&pageOffset, ((uint8_t*)&overflowPtr) + 4, 4);
    }

    template<typename T>
    static inline constexpr common::PhysicalTypeID getPhysicalTypeIDForType() {
        if constexpr (std::is_same_v<T, int64_t>) {
            return common::PhysicalTypeID::INT64;
        } else if constexpr (std::is_same_v<T, int32_t>) {
            return common::PhysicalTypeID::INT32;
        } else if constexpr (std::is_same_v<T, int16_t>) {
            return common::PhysicalTypeID::INT16;
        } else if constexpr (std::is_same_v<T, int8_t>) {
            return common::PhysicalTypeID::INT8;
        } else if constexpr (std::is_same_v<T, uint64_t>) {
            return common::PhysicalTypeID::UINT64;
        } else if constexpr (std::is_same_v<T, uint32_t>) {
            return common::PhysicalTypeID::UINT32;
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            return common::PhysicalTypeID::UINT16;
        } else if constexpr (std::is_same_v<T, uint8_t>) {
            return common::PhysicalTypeID::UINT8;
        } else if constexpr (std::is_same_v<T, float>) {
            return common::PhysicalTypeID::FLOAT;
        } else if constexpr (std::is_same_v<T, double>) {
            return common::PhysicalTypeID::DOUBLE;
        } else if constexpr (std::is_same_v<T, int128_t>) {
            return common::PhysicalTypeID::INT128;
        } else if constexpr (std::is_same_v<T, interval_t>) {
            return common::PhysicalTypeID::INTERVAL;
        } else if constexpr (std::same_as<T, ku_string_t> || std::same_as<T, std::string> ||
                             std::same_as<T, std::string_view>) {
            return common::PhysicalTypeID::STRING;
        } else {
            KU_UNREACHABLE;
        }
    }

    /*
     * TypeUtils::visit can be used to call generic code on all or some Logical and Physical type
     * variants with access to type information.
     *
     * E.g.
     *
     *  std::string result;
     *  visit(dataType, [&]<typename T>(T) {
     *      if constexpr(std::is_same_v<T, ku_string_t>()) {
     *          result = vector->getValue<ku_string_t>(0).getAsString();
     *      } else if (std::integral<T>) {
     *          result = std::to_string(vector->getValue<T>(0));
     *      } else {
     *          KU_UNREACHABLE;
     *      }
     *  });
     *
     * or
     *  std::string result;
     *  visit(dataType,
     *      [&](ku_string_t) {
     *          result = vector->getValue<ku_string_t>(0);
     *      },
     *      [&]<std::integral T>(T) {
     *          result = std::to_string(vector->getValue<T>(0));
     *      },
     *      [](auto) { KU_UNREACHABLE; }
     *  );
     *
     * Note that when multiple functions are provided, at least one function must match all data
     * types.
     *
     * Also note that implicit conversions may occur with the multi-function variant
     * if you don't include a generic auto function to cover types which aren't explicitly included.
     * See https://en.cppreference.com/w/cpp/utility/variant/visit
     */
    template<typename... Fs>
    static inline auto visit(const LogicalType& dataType, Fs... funcs) {
        // Note: arguments are used only for type deduction and have no meaningful value.
        // They should be optimized out by the compiler
        auto func = overload(funcs...);
        switch (dataType.getLogicalTypeID()) {
        /* NOLINTBEGIN(bugprone-branch-clone)*/
        case LogicalTypeID::INT8:
            return func(int8_t());
        case LogicalTypeID::UINT8:
            return func(uint8_t());
        case LogicalTypeID::INT16:
            return func(int16_t());
        case LogicalTypeID::UINT16:
            return func(uint16_t());
        case LogicalTypeID::INT32:
            return func(int32_t());
        case LogicalTypeID::UINT32:
            return func(uint32_t());
        case LogicalTypeID::SERIAL:
        case LogicalTypeID::INT64:
            return func(int64_t());
        case LogicalTypeID::UINT64:
            return func(uint64_t());
        case LogicalTypeID::BOOL:
            return func(bool());
        case LogicalTypeID::INT128:
            return func(int128_t());
        case LogicalTypeID::DOUBLE:
            return func(double());
        case LogicalTypeID::FLOAT:
            return func(float());
        case LogicalTypeID::DECIMAL:
            switch (dataType.getPhysicalType()) {
            case PhysicalTypeID::INT16:
                return func(int16_t());
            case PhysicalTypeID::INT32:
                return func(int32_t());
            case PhysicalTypeID::INT64:
                return func(int64_t());
            case PhysicalTypeID::INT128:
                return func(int128_t());
            default:
                KU_UNREACHABLE;
            }
        case LogicalTypeID::INTERVAL:
            return func(interval_t());
        case LogicalTypeID::INTERNAL_ID:
            return func(internalID_t());
        case LogicalTypeID::STRING:
            return func(ku_string_t());
        case LogicalTypeID::DATE:
            return func(date_t());
        case LogicalTypeID::TIMESTAMP_NS:
            return func(timestamp_ns_t());
        case LogicalTypeID::TIMESTAMP_MS:
            return func(timestamp_ms_t());
        case LogicalTypeID::TIMESTAMP_SEC:
            return func(timestamp_sec_t());
        case LogicalTypeID::TIMESTAMP_TZ:
            return func(timestamp_tz_t());
        case LogicalTypeID::TIMESTAMP:
            return func(timestamp_t());
        case LogicalTypeID::BLOB:
            return func(blob_t());
        case LogicalTypeID::UUID:
            return func(ku_uuid_t());
        case LogicalTypeID::ARRAY:
        case LogicalTypeID::LIST:
            return func(list_entry_t());
        case LogicalTypeID::MAP:
            return func(map_entry_t());
        case LogicalTypeID::NODE:
        case LogicalTypeID::REL:
        case LogicalTypeID::RECURSIVE_REL:
        case LogicalTypeID::STRUCT:
            return func(struct_entry_t());
        case LogicalTypeID::UNION:
            return func(union_entry_t());
        /* NOLINTEND(bugprone-branch-clone)*/
        case LogicalTypeID::ANY:
        case LogicalTypeID::POINTER:
        case LogicalTypeID::RDF_VARIANT:
            // Unsupported type
            KU_UNREACHABLE;
            // Needed for return type deduction to work
            return func(uint8_t());
        }
    }

    template<typename... Fs>
    static inline auto visit(PhysicalTypeID dataType, Fs&&... funcs) {
        // Note: arguments are used only for type deduction and have no meaningful value.
        // They should be optimized out by the compiler
        auto func = overload(funcs...);
        switch (dataType) {
        /* NOLINTBEGIN(bugprone-branch-clone)*/
        case PhysicalTypeID::INT8:
            return func(int8_t());
        case PhysicalTypeID::UINT8:
            return func(uint8_t());
        case PhysicalTypeID::INT16:
            return func(int16_t());
        case PhysicalTypeID::UINT16:
            return func(uint16_t());
        case PhysicalTypeID::INT32:
            return func(int32_t());
        case PhysicalTypeID::UINT32:
            return func(uint32_t());
        case PhysicalTypeID::INT64:
            return func(int64_t());
        case PhysicalTypeID::UINT64:
            return func(uint64_t());
        case PhysicalTypeID::BOOL:
            return func(bool());
        case PhysicalTypeID::INT128:
            return func(int128_t());
        case PhysicalTypeID::DOUBLE:
            return func(double());
        case PhysicalTypeID::FLOAT:
            return func(float());
        case PhysicalTypeID::INTERVAL:
            return func(interval_t());
        case PhysicalTypeID::INTERNAL_ID:
            return func(internalID_t());
        case PhysicalTypeID::STRING:
            return func(ku_string_t());
        case PhysicalTypeID::ARRAY:
        case PhysicalTypeID::LIST:
            return func(list_entry_t());
        case PhysicalTypeID::STRUCT:
            return func(struct_entry_t());
        /* NOLINTEND(bugprone-branch-clone)*/
        case PhysicalTypeID::ANY:
        case PhysicalTypeID::POINTER:
            // Unsupported type
            KU_UNREACHABLE;
            // Needed for return type deduction to work
            return func(uint8_t());
        default:
            KU_UNREACHABLE;
        }
    }
};

// Forward declaration of template specializations.
template<>
std::string TypeUtils::toString(const int128_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const bool& val, void* valueVector);
template<>
std::string TypeUtils::toString(const internalID_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const date_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const timestamp_ns_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const timestamp_ms_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const timestamp_sec_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const timestamp_tz_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const timestamp_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const interval_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const ku_string_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const blob_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const ku_uuid_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const list_entry_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const map_entry_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const struct_entry_t& val, void* valueVector);
template<>
std::string TypeUtils::toString(const union_entry_t& val, void* valueVector);

} // namespace common
} // namespace kuzu

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


namespace kuzu {
namespace main {

/**
 * @brief A prepared statement is a parameterized query which can avoid planning the same query for
 * repeated execution.
 */
class PreparedStatement {
    friend class Connection;
    friend class ClientContext;
    friend class testing::TestHelper;
    friend class testing::TestRunner;
    friend class testing::TinySnbDDLTest;
    friend class testing::TinySnbCopyCSVTransactionTest;

public:
    bool isTransactionStatement() const;
    /**
     * @return the query is prepared successfully or not.
     */
    KUZU_API bool isSuccess() const;
    /**
     * @return the error message if the query is not prepared successfully.
     */
    KUZU_API std::string getErrorMessage() const;
    /**
     * @return the prepared statement is read-only or not.
     */
    KUZU_API bool isReadOnly() const;

    inline std::unordered_map<std::string, std::shared_ptr<common::Value>> getParameterMap() {
        return parameterMap;
    }

    common::StatementType getStatementType();

    KUZU_API ~PreparedStatement();

private:
    bool isProfile();

private:
    bool success = true;
    bool readOnly = false;
    std::string errMsg;
    PreparedSummary preparedSummary;
    std::unordered_map<std::string, std::shared_ptr<common::Value>> parameterMap;
    std::unique_ptr<binder::BoundStatementResult> statementResult;
    std::vector<std::unique_ptr<planner::LogicalPlan>> logicalPlans;
    std::shared_ptr<parser::Statement> parsedStatement;
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace common {
class Serializer;
class Deserializer;
} // namespace common

namespace catalog {

class Property {
public:
    Property() = default;
    Property(std::string name, common::LogicalType dataType,
        std::unique_ptr<parser::ParsedExpression> defaultExpr)
        : Property{std::move(name), std::move(dataType), std::move(defaultExpr),
              common::INVALID_PROPERTY_ID, common::INVALID_COLUMN_ID, common::INVALID_TABLE_ID} {}
    Property(std::string name, common::LogicalType dataType,
        std::unique_ptr<parser::ParsedExpression> defaultExpr, common::property_id_t propertyID,
        common::column_id_t columnID, common::table_id_t tableID)
        : name{std::move(name)}, defaultExpr{std::move(defaultExpr)}, dataType{std::move(dataType)},
          propertyID{propertyID}, columnID{columnID}, tableID{tableID} {}
    EXPLICIT_COPY_DEFAULT_MOVE(Property);

    std::string getName() const { return name; }

    void setColumnID(common::column_id_t columnID) { this->columnID = columnID; }
    const common::LogicalType& getDataType() const { return dataType; }
    common::property_id_t getPropertyID() const { return propertyID; }
    common::column_id_t getColumnID() const { return columnID; }
    common::table_id_t getTableID() const { return tableID; }
    const parser::ParsedExpression* getDefaultExpr() const { return defaultExpr.get(); }

    void rename(std::string newName) { name = std::move(newName); }

    void serialize(common::Serializer& serializer) const;
    static Property deserialize(common::Deserializer& deserializer);

    static std::string toCypher(const std::vector<Property>& properties);

private:
    Property(const Property& other)
        : name{other.name}, defaultExpr{other.defaultExpr->copy()}, dataType{other.dataType.copy()},
          propertyID{other.propertyID}, columnID{other.columnID}, tableID{other.tableID} {}

private:
    std::string name;
    std::unique_ptr<parser::ParsedExpression> defaultExpr;
    common::LogicalType dataType;
    common::property_id_t propertyID;
    common::column_id_t columnID;
    common::table_id_t tableID;
};

} // namespace catalog
} // namespace kuzu

#include <memory>
#include <mutex>


namespace kuzu {
namespace common {

typedef std::unique_ptr<ProgressBarDisplay> (*progress_bar_display_create_func_t)();

/**
 * @brief Progress bar for tracking the progress of a pipeline. Prints the progress of each query
 * pipeline and the overall progress.
 */
class ProgressBar {
public:
    ProgressBar();

    static std::shared_ptr<ProgressBarDisplay> DefaultProgressBarDisplay();

    void addPipeline();

    void finishPipeline(uint64_t queryID);

    void endProgress(uint64_t queryID);

    void startProgress(uint64_t queryID);

    void toggleProgressBarPrinting(bool enable);

    void setShowProgressAfter(uint64_t showProgressAfter);

    void updateProgress(uint64_t queryID, double curPipelineProgress);

    void setDisplay(std::shared_ptr<ProgressBarDisplay> progressBarDipslay);

    std::shared_ptr<ProgressBarDisplay> getDisplay() { return display; }

    bool getProgressBarPrinting() const { return trackProgress; }

private:
    void resetProgressBar(uint64_t queryID);

    void updateDisplay(uint64_t queryID, double curPipelineProgress);

    bool shouldUpdateProgress() const;

private:
    uint32_t numPipelines;
    uint32_t numPipelinesFinished;
    std::mutex progressBarLock;
    bool trackProgress;
    std::unique_ptr<TimeMetric> queryTimer;
    uint64_t showProgressAfter;
    std::shared_ptr<ProgressBarDisplay> display;
};

} // namespace common
} // namespace kuzu

#include <utility>


namespace kuzu {
namespace common {

class NodeVal;
class RelVal;
struct FileInfo;
class NestedVal;
class RecursiveRelVal;
class ArrowRowBatch;
class ValueVector;
class Serializer;
class Deserializer;

class Value {
    friend class NodeVal;
    friend class RelVal;
    friend class NestedVal;
    friend class RecursiveRelVal;
    friend class ArrowRowBatch;
    friend class ValueVector;

public:
    /**
     * @return a NULL value of ANY type.
     */
    KUZU_API static Value createNullValue();
    /**
     * @param dataType the type of the NULL value.
     * @return a NULL value of the given type.
     */
    KUZU_API static Value createNullValue(const LogicalType& dataType);
    /**
     * @param dataType the type of the non-NULL value.
     * @return a default non-NULL value of the given type.
     */
    KUZU_API static Value createDefaultValue(const LogicalType& dataType);
    /**
     * @param val_ the boolean value to set.
     */
    KUZU_API explicit Value(bool val_);
    /**
     * @param val_ the int8_t value to set.
     */
    KUZU_API explicit Value(int8_t val_);
    /**
     * @param val_ the int16_t value to set.
     */
    KUZU_API explicit Value(int16_t val_);
    /**
     * @param val_ the int32_t value to set.
     */
    KUZU_API explicit Value(int32_t val_);
    /**
     * @param val_ the int64_t value to set.
     */
    KUZU_API explicit Value(int64_t val_);
    /**
     * @param val_ the uint8_t value to set.
     */
    KUZU_API explicit Value(uint8_t val_);
    /**
     * @param val_ the uint16_t value to set.
     */
    KUZU_API explicit Value(uint16_t val_);
    /**
     * @param val_ the uint32_t value to set.
     */
    KUZU_API explicit Value(uint32_t val_);
    /**
     * @param val_ the uint64_t value to set.
     */
    KUZU_API explicit Value(uint64_t val_);
    /**
     * @param val_ the int128_t value to set.
     */
    KUZU_API explicit Value(int128_t val_);
    /**
     * @param val_ the UUID value to set.
     */
    KUZU_API explicit Value(ku_uuid_t val_);
    /**
     * @param val_ the double value to set.
     */
    KUZU_API explicit Value(double val_);
    /**
     * @param val_ the float value to set.
     */
    KUZU_API explicit Value(float val_);
    /**
     * @param val_ the date value to set.
     */
    KUZU_API explicit Value(date_t val_);
    /**
     * @param val_ the timestamp_ns value to set.
     */
    KUZU_API explicit Value(timestamp_ns_t val_);
    /**
     * @param val_ the timestamp_ms value to set.
     */
    KUZU_API explicit Value(timestamp_ms_t val_);
    /**
     * @param val_ the timestamp_sec value to set.
     */
    KUZU_API explicit Value(timestamp_sec_t val_);
    /**
     * @param val_ the timestamp_tz value to set.
     */
    KUZU_API explicit Value(timestamp_tz_t val_);
    /**
     * @param val_ the timestamp value to set.
     */
    KUZU_API explicit Value(timestamp_t val_);
    /**
     * @param val_ the interval value to set.
     */
    KUZU_API explicit Value(interval_t val_);
    /**
     * @param val_ the internalID value to set.
     */
    KUZU_API explicit Value(internalID_t val_);
    /**
     * @param val_ the string value to set.
     */
    KUZU_API explicit Value(const char* val_);
    /**
     * @param val_ the string value to set.
     */
    KUZU_API explicit Value(const std::string& val_);
    /**
     * @param val_ the uint8_t* value to set.
     */
    KUZU_API explicit Value(uint8_t* val_);
    /**
     * @param type the logical type of the value.
     * @param val_ the string value to set.
     */
    KUZU_API explicit Value(LogicalType type, std::string val_);
    /**
     * @param dataType the logical type of the value.
     * @param children a vector of children values.
     */
    KUZU_API explicit Value(LogicalType dataType, std::vector<std::unique_ptr<Value>> children);
    /**
     * @param other the value to copy from.
     */
    KUZU_API Value(const Value& other);

    /**
     * @param other the value to move from.
     */
    KUZU_API Value(Value&& other) = default;
    KUZU_API Value& operator=(Value&& other) = default;
    KUZU_API bool operator==(const Value& rhs) const;

    /**
     * @brief Sets the data type of the Value.
     * @param dataType_ the data type to set to.
     */
    KUZU_API void setDataType(const LogicalType& dataType_);
    /**
     * @return the dataType of the value.
     */
    KUZU_API const LogicalType& getDataType() const;
    /**
     * @brief Sets the null flag of the Value.
     * @param flag null value flag to set.
     */
    KUZU_API void setNull(bool flag);
    /**
     * @brief Sets the null flag of the Value to true.
     */
    KUZU_API void setNull();
    /**
     * @return whether the Value is null or not.
     */
    KUZU_API bool isNull() const;
    /**
     * @brief Copies from the row layout value.
     * @param value value to copy from.
     */
    KUZU_API void copyFromRowLayout(const uint8_t* value);
    /**
     * @brief Copies from the col layout value.
     * @param value value to copy from.
     */
    KUZU_API void copyFromColLayout(const uint8_t* value, ValueVector* vec = nullptr);
    /**
     * @brief Copies from the other.
     * @param other value to copy from.
     */
    KUZU_API void copyValueFrom(const Value& other);
    /**
     * @return the value of the given type.
     */
    template<class T>
    T getValue() const {
        throw std::runtime_error("Unimplemented template for Value::getValue()");
    }
    /**
     * @return a reference to the value of the given type.
     */
    template<class T>
    T& getValueReference() {
        throw std::runtime_error("Unimplemented template for Value::getValueReference()");
    }
    /**
     * @return a Value object based on value.
     */
    template<class T>
    static Value createValue(T /*value*/) {
        throw std::runtime_error("Unimplemented template for Value::createValue()");
    }

    /**
     * @return a copy of the current value.
     */
    KUZU_API std::unique_ptr<Value> copy() const;
    /**
     * @return the current value in string format.
     */
    KUZU_API std::string toString() const;

    KUZU_API void serialize(Serializer& serializer) const;

    KUZU_API static std::unique_ptr<Value> deserialize(Deserializer& deserializer);

    void validateType(common::LogicalTypeID targetTypeID) const;

    bool hasNoneNullChildren() const;
    bool allowTypeChange() const;

    uint64_t computeHash() const;

    KUZU_API uint32_t getChildrenSize() const { return childrenSize; }

private:
    Value();
    explicit Value(const LogicalType& dataType);

    void resizeChildrenVector(uint64_t size, const LogicalType& childType);
    void copyFromRowLayoutList(const ku_list_t& list, const LogicalType& childType);
    void copyFromColLayoutList(const list_entry_t& list, ValueVector* vec);
    void copyFromRowLayoutStruct(const uint8_t* kuStruct);
    void copyFromColLayoutStruct(const struct_entry_t& structEntry, ValueVector* vec);
    void copyFromUnion(const uint8_t* kuUnion);

    std::string rdfVariantToString() const;
    std::string mapToString() const;
    std::string listToString() const;
    std::string structToString() const;
    std::string nodeToString() const;
    std::string relToString() const;
    std::string decimalToString() const;

public:
    union Val {
        constexpr Val() : booleanVal{false} {}
        bool booleanVal;
        int128_t int128Val;
        int64_t int64Val;
        int32_t int32Val;
        int16_t int16Val;
        int8_t int8Val;
        uint64_t uint64Val;
        uint32_t uint32Val;
        uint16_t uint16Val;
        uint8_t uint8Val;
        double doubleVal;
        float floatVal;
        // TODO(Ziyi): Should we remove the val suffix from all values in Val? Looks redundant.
        uint8_t* pointer;
        interval_t intervalVal;
        internalID_t internalIDVal;
    } val;
    std::string strVal;

private:
    LogicalType dataType;
    bool isNull_;

    // Note: ALWAYS use childrenSize over children.size(). We do NOT resize children when
    // iterating with nested value. So children.size() reflects the capacity() rather the actual
    // size.
    std::vector<std::unique_ptr<Value>> children;
    uint32_t childrenSize;
};

/**
 * @return boolean value.
 */
template<>
KUZU_API inline bool Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::BOOL);
    return val.booleanVal;
}

/**
 * @return int8 value.
 */
template<>
KUZU_API inline int8_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT8);
    return val.int8Val;
}

/**
 * @return int16 value.
 */
template<>
KUZU_API inline int16_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT16);
    return val.int16Val;
}

/**
 * @return int32 value.
 */
template<>
KUZU_API inline int32_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT32);
    return val.int32Val;
}

/**
 * @return int64 value.
 */
template<>
KUZU_API inline int64_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT64);
    return val.int64Val;
}

/**
 * @return uint64 value.
 */
template<>
KUZU_API inline uint64_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT64);
    return val.uint64Val;
}

/**
 * @return uint32 value.
 */
template<>
KUZU_API inline uint32_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT32);
    return val.uint32Val;
}

/**
 * @return uint16 value.
 */
template<>
KUZU_API inline uint16_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT16);
    return val.uint16Val;
}

/**
 * @return uint8 value.
 */
template<>
KUZU_API inline uint8_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT8);
    return val.uint8Val;
}

/**
 * @return int128 value.
 */
template<>
KUZU_API inline int128_t Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT128);
    return val.int128Val;
}

/**
 * @return float value.
 */
template<>
KUZU_API inline float Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::FLOAT);
    return val.floatVal;
}

/**
 * @return double value.
 */
template<>
KUZU_API inline double Value::getValue() const {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::DOUBLE);
    return val.doubleVal;
}

/**
 * @return date_t value.
 */
template<>
KUZU_API inline date_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::DATE);
    return date_t{val.int32Val};
}

/**
 * @return timestamp_t value.
 */
template<>
KUZU_API inline timestamp_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP);
    return timestamp_t{val.int64Val};
}

/**
 * @return timestamp_ns_t value.
 */
template<>
KUZU_API inline timestamp_ns_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_NS);
    return timestamp_ns_t{val.int64Val};
}

/**
 * @return timestamp_ms_t value.
 */
template<>
KUZU_API inline timestamp_ms_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_MS);
    return timestamp_ms_t{val.int64Val};
}

/**
 * @return timestamp_sec_t value.
 */
template<>
KUZU_API inline timestamp_sec_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_SEC);
    return timestamp_sec_t{val.int64Val};
}

/**
 * @return timestamp_tz_t value.
 */
template<>
KUZU_API inline timestamp_tz_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_TZ);
    return timestamp_tz_t{val.int64Val};
}

/**
 * @return interval_t value.
 */
template<>
KUZU_API inline interval_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::INTERVAL);
    return val.intervalVal;
}

/**
 * @return internal_t value.
 */
template<>
KUZU_API inline internalID_t Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::INTERNAL_ID);
    return val.internalIDVal;
}

/**
 * @return string value.
 */
template<>
KUZU_API inline std::string Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::STRING ||
              dataType.getLogicalTypeID() == LogicalTypeID::BLOB ||
              dataType.getLogicalTypeID() == LogicalTypeID::UUID);
    return strVal;
}

/**
 * @return uint8_t* value.
 */
template<>
KUZU_API inline uint8_t* Value::getValue() const {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::POINTER);
    return val.pointer;
}

/**
 * @return the reference to the boolean value.
 */
template<>
KUZU_API inline bool& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::BOOL);
    return val.booleanVal;
}

/**
 * @return the reference to the int8 value.
 */
template<>
KUZU_API inline int8_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT8);
    return val.int8Val;
}

/**
 * @return the reference to the int16 value.
 */
template<>
KUZU_API inline int16_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT16);
    return val.int16Val;
}

/**
 * @return the reference to the int32 value.
 */
template<>
KUZU_API inline int32_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT32);
    return val.int32Val;
}

/**
 * @return the reference to the int64 value.
 */
template<>
KUZU_API inline int64_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT64);
    return val.int64Val;
}

/**
 * @return the reference to the uint8 value.
 */
template<>
KUZU_API inline uint8_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT8);
    return val.uint8Val;
}

/**
 * @return the reference to the uint16 value.
 */
template<>
KUZU_API inline uint16_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT16);
    return val.uint16Val;
}

/**
 * @return the reference to the uint32 value.
 */
template<>
KUZU_API inline uint32_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT32);
    return val.uint32Val;
}

/**
 * @return the reference to the uint64 value.
 */
template<>
KUZU_API inline uint64_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::UINT64);
    return val.uint64Val;
}

/**
 * @return the reference to the int128 value.
 */
template<>
KUZU_API inline int128_t& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::INT128);
    return val.int128Val;
}

/**
 * @return the reference to the float value.
 */
template<>
KUZU_API inline float& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::FLOAT);
    return val.floatVal;
}

/**
 * @return the reference to the double value.
 */
template<>
KUZU_API inline double& Value::getValueReference() {
    KU_ASSERT(dataType.getPhysicalType() == PhysicalTypeID::DOUBLE);
    return val.doubleVal;
}

/**
 * @return the reference to the date value.
 */
template<>
KUZU_API inline date_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::DATE);
    return *reinterpret_cast<date_t*>(&val.int32Val);
}

/**
 * @return the reference to the timestamp value.
 */
template<>
KUZU_API inline timestamp_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP);
    return *reinterpret_cast<timestamp_t*>(&val.int64Val);
}

/**
 * @return the reference to the timestamp_ms value.
 */
template<>
KUZU_API inline timestamp_ms_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_MS);
    return *reinterpret_cast<timestamp_ms_t*>(&val.int64Val);
}

/**
 * @return the reference to the timestamp_ns value.
 */
template<>
KUZU_API inline timestamp_ns_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_NS);
    return *reinterpret_cast<timestamp_ns_t*>(&val.int64Val);
}

/**
 * @return the reference to the timestamp_sec value.
 */
template<>
KUZU_API inline timestamp_sec_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_SEC);
    return *reinterpret_cast<timestamp_sec_t*>(&val.int64Val);
}

/**
 * @return the reference to the timestamp_tz value.
 */
template<>
KUZU_API inline timestamp_tz_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::TIMESTAMP_TZ);
    return *reinterpret_cast<timestamp_tz_t*>(&val.int64Val);
}

/**
 * @return the reference to the interval value.
 */
template<>
KUZU_API inline interval_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::INTERVAL);
    return val.intervalVal;
}

/**
 * @return the reference to the internal_id value.
 */
template<>
KUZU_API inline nodeID_t& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::INTERNAL_ID);
    return val.internalIDVal;
}

/**
 * @return the reference to the string value.
 */
template<>
KUZU_API inline std::string& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::STRING);
    return strVal;
}

/**
 * @return the reference to the uint8_t* value.
 */
template<>
KUZU_API inline uint8_t*& Value::getValueReference() {
    KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::POINTER);
    return val.pointer;
}

/**
 * @param val the boolean value
 * @return a Value with BOOL type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(bool val) {
    return Value(val);
}

template<>
KUZU_API inline Value Value::createValue(int8_t val) {
    return Value(val);
}

/**
 * @param val the int16 value
 * @return a Value with INT16 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(int16_t val) {
    return Value(val);
}

/**
 * @param val the int32 value
 * @return a Value with INT32 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(int32_t val) {
    return Value(val);
}

/**
 * @param val the int64 value
 * @return a Value with INT64 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(int64_t val) {
    return Value(val);
}

/**
 * @param val the uint8 value
 * @return a Value with UINT8 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(uint8_t val) {
    return Value(val);
}

/**
 * @param val the uint16 value
 * @return a Value with UINT16 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(uint16_t val) {
    return Value(val);
}

/**
 * @param val the uint32 value
 * @return a Value with UINT32 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(uint32_t val) {
    return Value(val);
}

/**
 * @param val the uint64 value
 * @return a Value with UINT64 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(uint64_t val) {
    return Value(val);
}

/**
 * @param val the int128_t value
 * @return a Value with INT128 type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(int128_t val) {
    return Value(val);
}

/**
 * @param val the double value
 * @return a Value with DOUBLE type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(double val) {
    return Value(val);
}

/**
 * @param val the date_t value
 * @return a Value with DATE type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(date_t val) {
    return Value(val);
}

/**
 * @param val the timestamp_t value
 * @return a Value with TIMESTAMP type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(timestamp_t val) {
    return Value(val);
}

/**
 * @param val the interval_t value
 * @return a Value with INTERVAL type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(interval_t val) {
    return Value(val);
}

/**
 * @param val the nodeID_t value
 * @return a Value with NODE_ID type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(nodeID_t val) {
    return Value(val);
}

/**
 * @param val the string value
 * @return a Value with type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(std::string val) {
    return Value(LogicalType::STRING(), std::move(val));
}

/**
 * @param value the string value
 * @return a Value with STRING type and val value.
 */
template<>
KUZU_API inline Value Value::createValue(const char* value) {
    return Value(LogicalType::STRING(), std::string(value));
}

/**
 * @param val the uint8_t* val
 * @return a Value with POINTER type and val val.
 */
template<>
KUZU_API inline Value Value::createValue(uint8_t* val) {
    return Value(val);
}

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace common {

// F stands for Factorization
enum class FStateType : uint8_t {
    FLAT = 0,
    UNFLAT = 1,
};

class DataChunkState {
public:
    DataChunkState() : DataChunkState(DEFAULT_VECTOR_CAPACITY) {}
    explicit DataChunkState(sel_t capacity) : fStateType{FStateType::UNFLAT} {
        selVector = std::make_shared<SelectionVector>(capacity);
    }

    // returns a dataChunkState for vectors holding a single value.
    static std::shared_ptr<DataChunkState> getSingleValueDataChunkState();

    void initOriginalAndSelectedSize(uint64_t size) { selVector->setSelSize(size); }
    bool isFlat() const { return fStateType == FStateType::FLAT; }
    void setToFlat() { fStateType = FStateType::FLAT; }
    void setToUnflat() { fStateType = FStateType::UNFLAT; }

    const SelectionVector& getSelVector() const { return *selVector; }
    SelectionVector& getSelVectorUnsafe() { return *selVector; }
    std::shared_ptr<SelectionVector> getSelVectorShared() { return selVector; }
    void setSelVector(std::shared_ptr<SelectionVector> selVector_) {
        this->selVector = std::move(selVector_);
    }

    void slice(offset_t offset);

private:
    std::shared_ptr<SelectionVector> selVector;
    // TODO: We should get rid of `fStateType` and merge DataChunkState with SelectionVector.
    FStateType fStateType;
};

} // namespace common
} // namespace kuzu

#include <array>


namespace kuzu {
namespace transaction {
class Transaction;
} // namespace transaction

namespace storage {

struct VectorVersionInfo {
    enum class InsertionStatus : uint8_t { NO_INSERTED, CHECK_VERSION, ALWAYS_INSERTED };
    // TODO(Guodong): ALWAYS_INSERTED is not added for now, but it may be useful as an optimization
    // to mark the vector data after checkpoint is all deleted.
    enum class DeletionStatus : uint8_t { NO_DELETED, CHECK_VERSION };

    // TODO: Keep an additional same insertion/deletion field as an optimization to avoid the need
    // of `array` if all are inserted/deleted in the same transaction.
    // Also, avoid allocate `array` when status are NO_INSERTED and NO_DELETED.
    // We can even consider separating the insertion and deletion into two separate Vectors.
    std::array<common::transaction_t, common::DEFAULT_VECTOR_CAPACITY> insertedVersions;
    std::array<common::transaction_t, common::DEFAULT_VECTOR_CAPACITY> deletedVersions;
    InsertionStatus insertionStatus;
    DeletionStatus deletionStatus;

    VectorVersionInfo()
        : insertedVersions{}, deletedVersions{}, insertionStatus{InsertionStatus::NO_INSERTED},
          deletionStatus{DeletionStatus::NO_DELETED} {
        insertedVersions.fill(common::INVALID_TRANSACTION);
        deletedVersions.fill(common::INVALID_TRANSACTION);
    }
    DELETE_COPY_DEFAULT_MOVE(VectorVersionInfo);

    bool anyVersions() const {
        return insertionStatus == InsertionStatus::CHECK_VERSION ||
               deletionStatus == DeletionStatus::CHECK_VERSION;
    }
    common::row_idx_t append(common::transaction_t transactionID, common::row_idx_t startRow,
        common::row_idx_t numRows);
    bool delete_(common::transaction_t transactionID, common::row_idx_t rowIdx);

    void getSelVectorForScan(common::transaction_t startTS, common::transaction_t transactionID,
        common::SelectionVector& selVector, common::row_idx_t startRow, common::row_idx_t numRows,
        common::sel_t startOutputPos) const;

    void rollbackInsertions(common::row_idx_t startRowInVector, common::row_idx_t numRows);
    void rollbackDeletions(common::row_idx_t startRowInVector, common::row_idx_t numRows);

    void serialize(common::Serializer& serializer) const;
    static std::unique_ptr<VectorVersionInfo> deSerialize(common::Deserializer& deSer);

    common::row_idx_t numCommittedDeletions(const transaction::Transaction* transaction) const;

    // Given startTS and transactionID, if the row is deleted to the transaction, return true.
    bool isDeleted(common::transaction_t startTS, common::transaction_t transactionID,
        common::row_idx_t rowIdx) const;
    // Given startTS and transactionID, if the row is readable to the transaction, return true.
    bool isInserted(common::transaction_t startTS, common::transaction_t transactionID,
        common::row_idx_t rowIdx) const;

    common::row_idx_t getNumDeletions(common::transaction_t startTS,
        common::transaction_t transactionID, common::row_idx_t startRow,
        common::length_t numRows) const;

    // Return true if this vectorInfo needs to be kept after finalize.
    bool finalizeStatusFromVersions();
};

class VersionInfo {
public:
    VersionInfo() {}

    common::row_idx_t append(const transaction::Transaction* transaction,
        common::row_idx_t startRow, common::row_idx_t numRows);
    bool delete_(const transaction::Transaction* transaction, common::row_idx_t rowIdx);

    void getSelVectorToScan(common::transaction_t startTS, common::transaction_t transactionID,
        common::SelectionVector& selVector, common::row_idx_t startRow,
        common::row_idx_t numRows) const;

    void clearVectorInfo(common::idx_t vectorIdx);

    bool hasDeletions() const;
    common::row_idx_t getNumDeletions(const transaction::Transaction* transaction,
        common::row_idx_t startRow, common::length_t numRows) const;
    bool hasInsertions() const;
    bool isDeleted(const transaction::Transaction* transaction, common::row_idx_t rowInChunk) const;
    bool isInserted(const transaction::Transaction* transaction,
        common::row_idx_t rowInChunk) const;

    common::row_idx_t getNumDeletions(const transaction::Transaction* transaction) const;

    // Return nullptr when vectorIdx is out of range or when the vector is not created.
    VectorVersionInfo* getVectorVersionInfo(common::idx_t vectorIdx) const;
    common::idx_t getNumVectors() const { return vectorsInfo.size(); }
    VectorVersionInfo& getOrCreateVersionInfo(common::idx_t vectorIdx);

    bool finalizeStatusFromVersions();

    void serialize(common::Serializer& serializer) const;
    static std::unique_ptr<VersionInfo> deserialize(common::Deserializer& deSer);

private:
    std::vector<std::unique_ptr<VectorVersionInfo>> vectorsInfo;
};

} // namespace storage
} // namespace kuzu


namespace kuzu {

namespace main {
class ClientContext;
}

namespace function {

struct KUZU_API FunctionBindData {
    std::vector<common::LogicalType> paramTypes;
    common::LogicalType resultType;
    main::ClientContext* clientContext;
    int64_t count;

    explicit FunctionBindData(common::LogicalType dataType)
        : resultType{std::move(dataType)}, clientContext{nullptr}, count{1} {}
    FunctionBindData(std::vector<common::LogicalType> paramTypes, common::LogicalType resultType)
        : paramTypes{std::move(paramTypes)}, resultType{std::move(resultType)},
          clientContext{nullptr}, count{1} {}
    DELETE_COPY_AND_MOVE(FunctionBindData);
    virtual ~FunctionBindData() = default;

    static std::unique_ptr<FunctionBindData> getSimpleBindData(
        const binder::expression_vector& params, const common::LogicalType& resultType);

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<FunctionBindData&, TARGET&>(*this);
    }

    virtual std::unique_ptr<FunctionBindData> copy() const {
        return std::make_unique<FunctionBindData>(common::LogicalType::copy(paramTypes),
            resultType.copy());
    }
};

struct Function;
using function_set = std::vector<std::unique_ptr<Function>>;

struct ScalarBindFuncInput {
    const binder::expression_vector& arguments;
    Function* definition;
    main::ClientContext* context;

    ScalarBindFuncInput(const binder::expression_vector& expressionVectors, Function* definition,
        main::ClientContext* context)
        : arguments{expressionVectors}, definition{definition}, context{context} {}
};

using scalar_bind_func =
    std::function<std::unique_ptr<FunctionBindData>(ScalarBindFuncInput bindInput)>;

struct Function {
    std::string name;
    std::vector<common::LogicalTypeID> parameterTypeIDs;
    // Currently we only one variable-length function which is list creation. The expectation is
    // that all parameters must have the same type as parameterTypes[0].
    bool isVarLength;

    Function() : isVarLength{false} {};
    Function(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs)
        : name{std::move(name)}, parameterTypeIDs{std::move(parameterTypeIDs)}, isVarLength{false} {
    }
    Function(const Function&) = default;

    virtual ~Function() = default;

    virtual std::string signatureToString() const {
        return common::LogicalTypeUtils::toString(parameterTypeIDs);
    }

    virtual std::unique_ptr<Function> copy() const = 0;

    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const Function*, const TARGET*>(this);
    }
    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<Function*, TARGET*>(this);
    }
};

struct BaseScalarFunction : public Function {
    common::LogicalTypeID returnTypeID;
    scalar_bind_func bindFunc;

    BaseScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_bind_func bindFunc)
        : Function{std::move(name), std::move(parameterTypeIDs)}, returnTypeID{returnTypeID},
          bindFunc{std::move(bindFunc)} {}

    std::string signatureToString() const override {
        auto result = Function::signatureToString();
        result += " -> " + common::LogicalTypeUtils::toString(returnTypeID);
        return result;
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace binder {

struct BoundExtraAlterInfo {
    virtual ~BoundExtraAlterInfo() = default;

    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const BoundExtraAlterInfo*, const TARGET*>(this);
    }
    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const BoundExtraAlterInfo&, const TARGET&>(*this);
    }

    virtual std::unique_ptr<BoundExtraAlterInfo> copy() const = 0;
};

struct BoundAlterInfo {
    common::AlterType alterType;
    std::string tableName;
    common::table_id_t tableID;
    std::unique_ptr<BoundExtraAlterInfo> extraInfo;

    BoundAlterInfo(common::AlterType alterType, std::string tableName, common::table_id_t tableID,
        std::unique_ptr<BoundExtraAlterInfo> extraInfo)
        : alterType{alterType}, tableName{std::move(tableName)}, tableID{tableID},
          extraInfo{std::move(extraInfo)} {}
    EXPLICIT_COPY_DEFAULT_MOVE(BoundAlterInfo);

private:
    BoundAlterInfo(const BoundAlterInfo& other)
        : alterType{other.alterType}, tableName{other.tableName}, tableID{other.tableID},
          extraInfo{other.extraInfo->copy()} {}
};

struct BoundExtraRenameTableInfo : public BoundExtraAlterInfo {
    std::string newName;

    explicit BoundExtraRenameTableInfo(std::string newName) : newName{std::move(newName)} {}
    BoundExtraRenameTableInfo(const BoundExtraRenameTableInfo& other) : newName{other.newName} {}

    inline std::unique_ptr<BoundExtraAlterInfo> copy() const final {
        return std::make_unique<BoundExtraRenameTableInfo>(*this);
    }
};

struct BoundExtraAddPropertyInfo : public BoundExtraAlterInfo {
    std::string propertyName;
    common::LogicalType dataType;
    std::unique_ptr<parser::ParsedExpression> defaultValue;
    std::shared_ptr<Expression> boundDefault;

    BoundExtraAddPropertyInfo(std::string propertyName, common::LogicalType dataType,
        std::unique_ptr<parser::ParsedExpression> defaultValue,
        std::shared_ptr<Expression> boundDefault)
        : propertyName{std::move(propertyName)}, dataType{std::move(dataType)},
          defaultValue{std::move(defaultValue)}, boundDefault{std::move(boundDefault)} {}
    BoundExtraAddPropertyInfo(const BoundExtraAddPropertyInfo& other)
        : propertyName{other.propertyName}, dataType{other.dataType.copy()},
          defaultValue{other.defaultValue->copy()}, boundDefault{other.boundDefault} {}

    inline std::unique_ptr<BoundExtraAlterInfo> copy() const final {
        return std::make_unique<BoundExtraAddPropertyInfo>(*this);
    }
};

struct BoundExtraDropPropertyInfo : public BoundExtraAlterInfo {
    common::property_id_t propertyID;
    std::string propertyName;

    explicit BoundExtraDropPropertyInfo(common::property_id_t propertyID, std::string propertyName)
        : propertyID{propertyID}, propertyName{propertyName} {}
    BoundExtraDropPropertyInfo(const BoundExtraDropPropertyInfo& other)
        : propertyID{other.propertyID}, propertyName{other.propertyName} {}

    inline std::unique_ptr<BoundExtraAlterInfo> copy() const final {
        return std::make_unique<BoundExtraDropPropertyInfo>(*this);
    }
};

struct BoundExtraRenamePropertyInfo : public BoundExtraAlterInfo {
    common::property_id_t propertyID;
    std::string newName;
    std::string oldName;

    BoundExtraRenamePropertyInfo(common::property_id_t propertyID, std::string newName,
        std::string oldName)
        : propertyID{propertyID}, newName{std::move(newName)}, oldName{std::move(oldName)} {}
    BoundExtraRenamePropertyInfo(const BoundExtraRenamePropertyInfo& other)
        : propertyID{other.propertyID}, newName{other.newName}, oldName{other.oldName} {}
    inline std::unique_ptr<BoundExtraAlterInfo> copy() const final {
        return std::make_unique<BoundExtraRenamePropertyInfo>(*this);
    }
};

struct BoundExtraCommentInfo : public BoundExtraAlterInfo {
    std::string comment;

    explicit BoundExtraCommentInfo(std::string comment) : comment{std::move(comment)} {}
    BoundExtraCommentInfo(const BoundExtraCommentInfo& other) : comment{other.comment} {}
    inline std::unique_ptr<BoundExtraAlterInfo> copy() const final {
        return std::make_unique<BoundExtraCommentInfo>(*this);
    }
};

} // namespace binder
} // namespace kuzu


namespace kuzu {
namespace storage {

struct CompressionMetadata;

class ColumnPredicate;
class ColumnPredicateSet {
public:
    ColumnPredicateSet() = default;
    EXPLICIT_COPY_DEFAULT_MOVE(ColumnPredicateSet);

    void addPredicate(std::unique_ptr<ColumnPredicate> predicate) {
        predicates.push_back(std::move(predicate));
    }

    common::ZoneMapCheckResult checkZoneMap(const CompressionMetadata& metadata);

private:
    ColumnPredicateSet(const ColumnPredicateSet& other);

private:
    std::vector<std::unique_ptr<ColumnPredicate>> predicates;
};

class ColumnPredicate {
public:
    virtual ~ColumnPredicate() = default;

    virtual common::ZoneMapCheckResult checkZoneMap(const CompressionMetadata& metadata) const = 0;

    virtual std::unique_ptr<ColumnPredicate> copy() const = 0;

    template<class TARGET>
    const TARGET& constCast() const {
        return common::ku_dynamic_cast<const ColumnPredicate&, const TARGET&>(*this);
    }
};

struct ColumnPredicateUtil {
    static std::unique_ptr<ColumnPredicate> tryConvert(const binder::Expression& property,
        const binder::Expression& predicate);
};

} // namespace storage
} // namespace kuzu

#include <mutex>


namespace kuzu {
namespace binder {
struct BoundAlterInfo;
} // namespace binder

namespace storage {
class UndoBuffer;
} // namespace storage

namespace transaction {
class Transaction;
} // namespace transaction

using CatalogEntrySet = common::case_insensitive_map_t<catalog::CatalogEntry*>;

namespace catalog {
class CatalogSet {
    friend class storage::UndoBuffer;

public:
    //===--------------------------------------------------------------------===//
    // getters & setters
    //===--------------------------------------------------------------------===//
    bool containsEntry(transaction::Transaction* transaction, const std::string& name);
    CatalogEntry* getEntry(transaction::Transaction* transaction, const std::string& name);
    KUZU_API void createEntry(transaction::Transaction* transaction,
        std::unique_ptr<CatalogEntry> entry);
    void dropEntry(transaction::Transaction* transaction, const std::string& name);
    void alterEntry(transaction::Transaction* transaction, const binder::BoundAlterInfo& alterInfo);
    CatalogEntrySet getEntries(transaction::Transaction* transaction);

    uint64_t assignNextOID() {
        std::lock_guard lck{mtx};
        return nextOID++;
    }

    //===--------------------------------------------------------------------===//
    // serialization & deserialization
    //===--------------------------------------------------------------------===//
    void serialize(common::Serializer serializer) const;
    static std::unique_ptr<CatalogSet> deserialize(common::Deserializer& deserializer);

private:
    bool containsEntryNoLock(transaction::Transaction* transaction, const std::string& name) const;
    CatalogEntry* getEntryNoLock(transaction::Transaction* transaction, const std::string& name);
    CatalogEntry* createEntryNoLock(transaction::Transaction* transaction,
        std::unique_ptr<CatalogEntry> entry);
    CatalogEntry* dropEntryNoLock(transaction::Transaction* transaction, const std::string& name);

    void validateExistNoLock(transaction::Transaction* transaction, const std::string& name) const;
    void validateNotExistNoLock(transaction::Transaction* transaction,
        const std::string& name) const;

    void emplaceNoLock(std::unique_ptr<CatalogEntry> entry);
    void eraseNoLock(const std::string& name);

    std::unique_ptr<CatalogEntry> createDummyEntryNoLock(std::string name) const;

    CatalogEntry* traverseVersionChainsForTransactionNoLock(transaction::Transaction* transaction,
        CatalogEntry* currentEntry) const;
    CatalogEntry* getCommittedEntryNoLock(CatalogEntry* entry) const;

private:
    std::mutex mtx;
    uint64_t nextOID = 0;
    common::case_insensitive_map_t<std::unique_ptr<CatalogEntry>> entries;
};

} // namespace catalog
} // namespace kuzu

#include <cstdint>
#include <cstring>
#include <optional>
#include <type_traits>

#include <concepts>
#include <span>

namespace kuzu {
namespace common {
class ValueVector;
class NullMask;
} // namespace common

namespace storage {
class ColumnChunkData;

struct PageCursor;

template<typename T>
concept StorageValueType = (common::numeric_utils::IsIntegral<T> || std::floating_point<T>);
// Type storing values in the column chunk statistics
// Only supports integers (up to 128bit), floats and bools
union StorageValue {
    int64_t signedInt;
    uint64_t unsignedInt;
    double floatVal;
    common::int128_t signedInt128;

    StorageValue() = default;
    template<typename T>
        requires std::same_as<std::remove_cvref_t<T>, common::int128_t>
    explicit StorageValue(T value) : signedInt128(value) {}

    template<typename T>
        requires std::integral<T> && std::numeric_limits<T>::is_signed
    // zero-initilize union padding
    explicit StorageValue(T value) : StorageValue(common::int128_t(0)) {
        signedInt = value;
    }

    template<typename T>
        requires std::integral<T> && (!std::numeric_limits<T>::is_signed)
    explicit StorageValue(T value) : StorageValue(common::int128_t(0)) {
        unsignedInt = value;
    }

    template<typename T>
        requires std::is_floating_point<T>::value
    explicit StorageValue(T value) : StorageValue(common::int128_t(0)) {
        floatVal = value;
    }

    bool operator==(const StorageValue& other) const {
        // All types are the same size, so we can compare any of them to check equality
        return this->signedInt == other.signedInt;
    }

    template<StorageValueType T>
    StorageValue& operator=(const T& val) {
        return *this = StorageValue(val);
    }

    template<StorageValueType T>
    T get() const {
        if constexpr (std::same_as<std::remove_cvref_t<T>, common::int128_t>) {
            return signedInt128;
        } else if constexpr (std::integral<T>) {
            if constexpr (std::numeric_limits<T>::is_signed) {
                return static_cast<T>(signedInt);
            } else {
                return static_cast<T>(unsignedInt);
            }
        } else if constexpr (std::is_floating_point<T>()) {
            return floatVal;
        }
    }

    bool gt(const StorageValue& other, common::PhysicalTypeID type) const;

    // If the type cannot be stored in the statistics, readFromVector will return nullopt
    static std::optional<StorageValue> readFromVector(const common::ValueVector& vector,
        common::offset_t posInVector);
};
static_assert(std::is_trivial_v<StorageValue>);

std::pair<std::optional<StorageValue>, std::optional<StorageValue>> getMinMaxStorageValue(
    const uint8_t* data, uint64_t offset, uint64_t numValues, common::PhysicalTypeID physicalType,
    const common::NullMask* nullMask, bool valueRequiredIfUnsupported = false);

// Returns the size of the data type in bytes
uint32_t getDataTypeSizeInChunk(const common::LogicalType& dataType);
uint32_t getDataTypeSizeInChunk(const common::PhysicalTypeID& dataType);

// Compression type is written to the data header both so we can usually catch issues when we
// decompress uncompressed data by mistake, and to allow for runtime-configurable compression.
enum class CompressionType : uint8_t {
    UNCOMPRESSED = 0,
    INTEGER_BITPACKING = 1,
    BOOLEAN_BITPACKING = 2,
    CONSTANT = 3,
};

// Data statistics used for determining how to handle compressed data
struct CompressionMetadata {
    // Minimum and maximum are upper and lower bounds for the data.
    // Updates and deletions may cause them to no longer be the exact minimums and maximums,
    // but no value will be larger than the maximum or smaller than the minimum
    StorageValue min;
    StorageValue max;
    CompressionType compression;
    uint8_t _padding[7]{};

    CompressionMetadata(StorageValue min, StorageValue max, CompressionType compression)
        : min(min), max(max), compression(compression) {}
    inline bool isConstant() const { return compression == CompressionType::CONSTANT; }

    // Returns the number of values which will be stored in the given data size
    // This must be consistent with the compression implementation for the given size
    uint64_t numValues(uint64_t dataSize, const common::LogicalType& dataType) const;
    // Returns true if and only if the provided value within the vector can be updated
    // in this chunk in-place.
    bool canUpdateInPlace(const uint8_t* data, uint32_t pos, uint64_t numValues,
        common::PhysicalTypeID physicalType,
        const std::optional<common::NullMask>& nullMask = std::nullopt) const;
    bool canAlwaysUpdateInPlace() const;

    std::string toString(const common::PhysicalTypeID physicalType) const;
};
// Padding should be kept to a minimum, but must be stored explicitly for consistent binary output
// when writing the padding to disk.
static_assert(sizeof(CompressionMetadata) == sizeof(StorageValue) * 2 + 8);

class CompressionAlg {
public:
    virtual ~CompressionAlg() = default;

    // Takes a single uncompressed value from the srcBuffer and compresses it into the dstBuffer
    // Offsets refer to value offsets, not byte offsets
    //
    // nullMask may be null if no mask is available (all values are non-null)
    // Storage of null values is handled by the implementation and decompression of null values
    // does not have to produce the original value passed to this function.
    virtual void setValuesFromUncompressed(const uint8_t* srcBuffer, common::offset_t srcOffset,
        uint8_t* dstBuffer, common::offset_t dstOffset, common::offset_t numValues,
        const CompressionMetadata& metadata, const common::NullMask* nullMask) const = 0;

    // Takes uncompressed data from the srcBuffer and compresses it into the dstBuffer
    //
    // stores only as much data in dstBuffer as will fit, and advances the srcBuffer pointer
    // to the beginning of the next value to store.
    // (This means that we can't start the next page on an unaligned value.
    // Maybe instead we could use value offsets, but the compression algorithms
    // usually work on aligned chunks anyway)
    //
    // dstBufferSize is the size in bytes
    // numValuesRemaining is the number of values remaining in the srcBuffer to be compressed.
    //      compressNextPage must store the least of either the number of values per page
    //      (as calculated by CompressionMetadata::numValues), or the remaining number of values.
    //
    // returns the size in bytes of the compressed data within the page (rounded up to the nearest
    // byte)
    virtual uint64_t compressNextPage(const uint8_t*& srcBuffer, uint64_t numValuesRemaining,
        uint8_t* dstBuffer, uint64_t dstBufferSize,
        const struct CompressionMetadata& metadata) const = 0;

    // Takes compressed data from the srcBuffer and decompresses it into the dstBuffer
    // Offsets refer to value offsets, not byte offsets
    // srcBuffer points to the beginning of a page
    virtual void decompressFromPage(const uint8_t* srcBuffer, uint64_t srcOffset,
        uint8_t* dstBuffer, uint64_t dstOffset, uint64_t numValues,
        const CompressionMetadata& metadata) const = 0;

    virtual CompressionType getCompressionType() const = 0;
};

class ConstantCompression final : public CompressionAlg {
public:
    explicit ConstantCompression(const common::LogicalType& logicalType)
        : numBytesPerValue{static_cast<uint8_t>(getDataTypeSizeInChunk(logicalType))},
          dataType{logicalType.getPhysicalType()} {}
    static std::optional<CompressionMetadata> analyze(const ColumnChunkData& chunk);

    // Shouldn't be used, there's a special case when compressing which ends early for constant
    // compression
    uint64_t compressNextPage(const uint8_t*&, uint64_t, uint8_t*, uint64_t,
        const struct CompressionMetadata&) const override {
        return 0;
    };

    static void decompressValues(uint8_t* dstBuffer, uint64_t dstOffset, uint64_t numValues,
        common::PhysicalTypeID physicalType, uint32_t numBytesPerValue,
        const CompressionMetadata& metadata);

    void decompressFromPage(const uint8_t* /*srcBuffer*/, uint64_t /*srcOffset*/,
        uint8_t* dstBuffer, uint64_t dstOffset, uint64_t numValues,
        const CompressionMetadata& metadata) const override;

    void copyFromPage(const uint8_t* /*srcBuffer*/, uint64_t /*srcOffset*/, uint8_t* dstBuffer,
        uint64_t dstOffset, uint64_t numValues, const CompressionMetadata& metadata) const;

    // Nothing to do; constant compressed data is only updated if the update is to the same value
    void setValuesFromUncompressed(const uint8_t*, common::offset_t, uint8_t*, common::offset_t,
        common::offset_t, const CompressionMetadata&,
        const common::NullMask* /*nullMask*/) const override {};

    CompressionType getCompressionType() const override { return CompressionType::CONSTANT; }

private:
    uint8_t numBytesPerValue;
    common::PhysicalTypeID dataType;
};

// Compression alg which does not compress values and instead just copies them.
class Uncompressed : public CompressionAlg {
public:
    explicit Uncompressed(const common::LogicalType& logicalType)
        : numBytesPerValue{getDataTypeSizeInChunk(logicalType)} {}
    explicit Uncompressed(uint8_t numBytesPerValue) : numBytesPerValue{numBytesPerValue} {}

    Uncompressed(const Uncompressed&) = default;

    inline void setValuesFromUncompressed(const uint8_t* srcBuffer, common::offset_t srcOffset,
        uint8_t* dstBuffer, common::offset_t dstOffset, common::offset_t numValues,
        const CompressionMetadata& /*metadata*/, const common::NullMask* /*nullMask*/) const final {
        memcpy(dstBuffer + dstOffset * numBytesPerValue, srcBuffer + srcOffset * numBytesPerValue,
            numBytesPerValue * numValues);
    }

    static inline uint64_t numValues(uint64_t dataSize, const common::LogicalType& logicalType) {
        auto numBytesPerValue = getDataTypeSizeInChunk(logicalType);
        return numBytesPerValue == 0 ? UINT64_MAX : dataSize / numBytesPerValue;
    }

    inline uint64_t compressNextPage(const uint8_t*& srcBuffer, uint64_t numValuesRemaining,
        uint8_t* dstBuffer, uint64_t dstBufferSize,
        const struct CompressionMetadata& /*metadata*/) const override {
        if (numBytesPerValue == 0) {
            return 0;
        }
        uint64_t numValues = std::min(numValuesRemaining, dstBufferSize / numBytesPerValue);
        uint64_t sizeToCopy = numValues * numBytesPerValue;
        KU_ASSERT(sizeToCopy <= dstBufferSize);
        std::memcpy(dstBuffer, srcBuffer, sizeToCopy);
        srcBuffer += sizeToCopy;
        return sizeToCopy;
    }

    inline void decompressFromPage(const uint8_t* srcBuffer, uint64_t srcOffset, uint8_t* dstBuffer,
        uint64_t dstOffset, uint64_t numValues,
        const CompressionMetadata& /*metadata*/) const override {
        std::memcpy(dstBuffer + dstOffset * numBytesPerValue,
            srcBuffer + srcOffset * numBytesPerValue, numValues * numBytesPerValue);
    }

    CompressionType getCompressionType() const override { return CompressionType::UNCOMPRESSED; }

protected:
    const uint32_t numBytesPerValue;
};

template<typename T>
struct BitpackInfo {
    uint8_t bitWidth;
    bool hasNegative;
    T offset;
};

template<typename T>
concept IntegerBitpackingType = (common::numeric_utils::IsIntegral<T> && !std::same_as<T, bool>);

// Augmented with Frame of Reference encoding using an offset stored in the compression metadata
template<IntegerBitpackingType T>
class IntegerBitpacking : public CompressionAlg {
    using U = common::numeric_utils::MakeUnSignedT<T>;

public:
    // This is an implementation detail of the fastpfor bitpacking algorithm
    static constexpr uint64_t CHUNK_SIZE = 32;

public:
    IntegerBitpacking() = default;
    IntegerBitpacking(const IntegerBitpacking&) = default;

    void setValuesFromUncompressed(const uint8_t* srcBuffer, common::offset_t srcOffset,
        uint8_t* dstBuffer, common::offset_t dstOffset, common::offset_t numValues,
        const CompressionMetadata& metadata, const common::NullMask* nullMask) const final;

    static BitpackInfo<T> getPackingInfo(const CompressionMetadata& metadata);

    static inline uint64_t numValues(uint64_t dataSize, const BitpackInfo<T>& info) {
        if (info.bitWidth == 0) {
            return UINT64_MAX;
        }
        auto numValues = dataSize * 8 / info.bitWidth;
        return numValues;
    }

    static inline uint64_t numValues(uint64_t dataSize, const CompressionMetadata& metadata) {
        auto info = getPackingInfo(metadata);
        return numValues(dataSize, info);
    }

    uint64_t compressNextPage(const uint8_t*& srcBuffer, uint64_t numValuesRemaining,
        uint8_t* dstBuffer, uint64_t dstBufferSize,
        const struct CompressionMetadata& metadata) const final;

    void decompressFromPage(const uint8_t* srcBuffer, uint64_t srcOffset, uint8_t* dstBuffer,
        uint64_t dstOffset, uint64_t numValues,
        const struct CompressionMetadata& metadata) const final;

    static bool canUpdateInPlace(std::span<T> value, const CompressionMetadata& metadata,
        const std::optional<common::NullMask>& nullMask = std::nullopt,
        uint64_t nullMaskOffset = 0);

    CompressionType getCompressionType() const override {
        return CompressionType::INTEGER_BITPACKING;
    }

protected:
    // Read multiple values from within a chunk. Cannot span multiple chunks.
    void getValues(const uint8_t* chunkStart, uint8_t pos, uint8_t* dst, uint8_t numValuesToRead,
        const BitpackInfo<T>& header) const;

    inline const uint8_t* getChunkStart(const uint8_t* buffer, uint64_t pos,
        uint8_t bitWidth) const {
        // Order of operations is important so that pos is rounded down to a multiple of
        // CHUNK_SIZE
        return buffer + (pos / CHUNK_SIZE) * bitWidth * CHUNK_SIZE / 8;
    }

    void packPartialChunk(const U* srcBuffer, uint8_t* dstBuffer, size_t posInDst,
        BitpackInfo<T> info, size_t remainingValues) const;

    void copyValuesToTempChunkWithOffset(const U* srcBuffer, U* tmpBuffer, BitpackInfo<T> info,
        size_t numValuesToCopy) const;

    void setPartialChunkInPlace(const uint8_t* srcBuffer, common::offset_t posInSrc,
        uint8_t* dstBuffer, common::offset_t posInDst, common::offset_t numValues,
        const BitpackInfo<T>& header) const;
};

class BooleanBitpacking : public CompressionAlg {
public:
    BooleanBitpacking() = default;
    BooleanBitpacking(const BooleanBitpacking&) = default;

    void setValuesFromUncompressed(const uint8_t* srcBuffer, common::offset_t srcOffset,
        uint8_t* dstBuffer, common::offset_t dstOffset, common::offset_t numValues,
        const CompressionMetadata& metadata, const common::NullMask* nullMask) const final;

    static inline uint64_t numValues(uint64_t dataSize) { return dataSize * 8; }

    uint64_t compressNextPage(const uint8_t*& srcBuffer, uint64_t numValuesRemaining,
        uint8_t* dstBuffer, uint64_t dstBufferSize,
        const struct CompressionMetadata& metadata) const final;

    void decompressFromPage(const uint8_t* srcBuffer, uint64_t srcOffset, uint8_t* dstBuffer,
        uint64_t dstOffset, uint64_t numValues, const CompressionMetadata& metadata) const final;

    void copyFromPage(const uint8_t* srcBuffer, uint64_t srcOffset, uint8_t* dstBuffer,
        uint64_t dstOffset, uint64_t numValues, const CompressionMetadata& metadata) const;

    CompressionType getCompressionType() const override {
        return CompressionType::BOOLEAN_BITPACKING;
    }
};

class CompressedFunctor {
public:
    CompressedFunctor(const CompressedFunctor&) = default;

protected:
    explicit CompressedFunctor(const common::LogicalType& logicalType)
        : constant{logicalType}, uncompressed{logicalType},
          physicalType{logicalType.getPhysicalType()} {}
    const ConstantCompression constant;
    const Uncompressed uncompressed;
    const BooleanBitpacking booleanBitpacking;
    const common::PhysicalTypeID physicalType;
};

class ReadCompressedValuesFromPageToVector : public CompressedFunctor {
public:
    explicit ReadCompressedValuesFromPageToVector(const common::LogicalType& logicalType)
        : CompressedFunctor(logicalType) {}
    ReadCompressedValuesFromPageToVector(const ReadCompressedValuesFromPageToVector&) = default;

    void operator()(const uint8_t* frame, PageCursor& pageCursor, common::ValueVector* resultVector,
        uint32_t posInVector, uint32_t numValuesToRead, const CompressionMetadata& metadata);
};

class ReadCompressedValuesFromPage : public CompressedFunctor {
public:
    explicit ReadCompressedValuesFromPage(const common::LogicalType& logicalType)
        : CompressedFunctor(logicalType) {}
    ReadCompressedValuesFromPage(const ReadCompressedValuesFromPage&) = default;

    void operator()(const uint8_t* frame, PageCursor& pageCursor, uint8_t* result,
        uint32_t startPosInResult, uint64_t numValuesToRead, const CompressionMetadata& metadata);
};

class WriteCompressedValuesToPage : public CompressedFunctor {
public:
    explicit WriteCompressedValuesToPage(const common::LogicalType& logicalType)
        : CompressedFunctor(logicalType) {}
    WriteCompressedValuesToPage(const WriteCompressedValuesToPage&) = default;

    void operator()(uint8_t* frame, uint16_t posInFrame, const uint8_t* data,
        common::offset_t dataOffset, common::offset_t numValues,
        const CompressionMetadata& metadata, const common::NullMask* nullMask = nullptr);

    void operator()(uint8_t* frame, uint16_t posInFrame, common::ValueVector* vector,
        uint32_t posInVector, const CompressionMetadata& metadata);
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace common {
class Value;
class RdfVariant {
public:
    /**
     * @brief Get the logical type id of the rdf variant.
     * @param rdfVariant the rdf variant.
     * @return the logical type id.
     */
    KUZU_API static LogicalTypeID getLogicalTypeID(const Value* rdfVariant);

    /**
     * @brief Get the value of the rdf variant.
     * @tparam T the type of the value.
     * @param rdfVariant the rdf variant.
     * @return the value.
     */
    template<typename T>
    T static getValue(const Value* rdfVariant) {
        auto blobData = NestedVal::getChildVal(rdfVariant, 1)->strVal.data();
        return Blob::getValue<T>(blobData);
    }
};

/**
 * @brief Specialization for string.
 * @param rdfVariant the rdf variant.
 * @return the string value.
 */
template<>
KUZU_API inline std::string RdfVariant::getValue<std::string>(const Value* rdfVariant) {
    return NestedVal::getChildVal(rdfVariant, 1)->strVal;
}
} // namespace common
} // namespace kuzu

#include <string>
#include <vector>


namespace kuzu {
namespace common {

enum class FileType : uint8_t {
    UNKNOWN = 0,
    CSV = 1,
    PARQUET = 2,
    NPY = 3,
    TURTLE = 4,   // Terse triples http://www.w3.org/TR/turtle
    NQUADS = 5,   // Line-based quads http://www.w3.org/TR/n-quads/
    NTRIPLES = 6, // Line-based triples http://www.w3.org/TR/n-triples/
};

struct FileTypeInfo {
    FileType fileType;
    std::string fileTypeStr;
};

struct FileTypeUtils {
    static FileType getFileTypeFromExtension(std::string_view extension);
    static std::string toString(FileType fileType);
    static FileType fromString(std::string fileType);
};

struct ReaderConfig {
    FileTypeInfo fileTypeInfo;
    std::vector<std::string> filePaths;
    std::unordered_map<std::string, Value> options;

    ReaderConfig() : fileTypeInfo{FileType::UNKNOWN, ""} {}
    ReaderConfig(FileTypeInfo fileTypeInfo, std::vector<std::string> filePaths)
        : fileTypeInfo{std::move(fileTypeInfo)}, filePaths{std::move(filePaths)} {}
    EXPLICIT_COPY_DEFAULT_MOVE(ReaderConfig);

    uint32_t getNumFiles() const { return filePaths.size(); }

private:
    ReaderConfig(const ReaderConfig& other)
        : fileTypeInfo{other.fileTypeInfo}, filePaths{other.filePaths}, options{other.options} {}
};

} // namespace common
} // namespace kuzu

#include <cstdint>
#include <memory>
#include <string>
#include <vector>


namespace kuzu {
namespace processor {

/**
 * @brief Stores a vector of Values.
 */
class FlatTuple {
public:
    void addValue(std::unique_ptr<common::Value> value);

    /**
     * @return number of values in the FlatTuple.
     */
    KUZU_API uint32_t len() const;

    /**
     * @param idx value index to get.
     * @return the value stored at idx.
     */
    KUZU_API common::Value* getValue(uint32_t idx) const;

    KUZU_API std::string toString();

    /**
     * @param colsWidth The length of each column
     * @param delimiter The delimiter to separate each value.
     * @param maxWidth The maximum length of each column. Only the first maxWidth number of
     * characters of each column will be displayed.
     * @return all values in string format.
     */
    KUZU_API std::string toString(const std::vector<uint32_t>& colsWidth,
        const std::string& delimiter = "|", uint32_t maxWidth = -1);

private:
    std::vector<std::unique_ptr<common::Value>> values;
};

} // namespace processor
} // namespace kuzu


namespace kuzu {
namespace parser {

class ParsedLiteralExpression : public ParsedExpression {
    static constexpr common::ExpressionType expressionType = common::ExpressionType::LITERAL;

public:
    ParsedLiteralExpression(common::Value value, std::string raw)
        : ParsedExpression{expressionType, std::move(raw)}, value{std::move(value)} {}

    ParsedLiteralExpression(std::string alias, std::string rawName, parsed_expr_vector children,
        common::Value value)
        : ParsedExpression{expressionType, std::move(alias), std::move(rawName),
              std::move(children)},
          value{std::move(value)} {}

    explicit ParsedLiteralExpression(common::Value value)
        : ParsedExpression{expressionType}, value{std::move(value)} {}

    common::Value getValue() const { return value; }

    common::Value& getValueUnsafe() { return value; }

    static std::unique_ptr<ParsedLiteralExpression> deserialize(
        common::Deserializer& deserializer) {
        return std::make_unique<ParsedLiteralExpression>(*common::Value::deserialize(deserializer));
    }

    std::unique_ptr<ParsedExpression> copy() const override {
        return std::make_unique<ParsedLiteralExpression>(alias, rawName, copyVector(children),
            value);
    }

private:
    void serializeInternal(common::Serializer& serializer) const override {
        value.serialize(serializer);
    }

private:
    common::Value value;
};

} // namespace parser
} // namespace kuzu

#include <string>


namespace kuzu {
namespace common {
class Value;
enum class LogicalTypeID : uint8_t;
} // namespace common

namespace main {

class ClientContext;
struct SystemConfig;

typedef void (*set_context)(ClientContext* context, const common::Value& parameter);
typedef common::Value (*get_setting)(const ClientContext* context);

enum class OptionType : uint8_t { CONFIGURATION = 0, EXTENSION = 1 };

struct Option {
    std::string name;
    common::LogicalTypeID parameterType;
    OptionType optionType;

    Option(std::string name, common::LogicalTypeID parameterType, OptionType optionType)
        : name{std::move(name)}, parameterType{std::move(parameterType)}, optionType{optionType} {}

    virtual ~Option() = default;
};

struct ConfigurationOption final : Option {
    set_context setContext;
    get_setting getSetting;

    ConfigurationOption(std::string name, common::LogicalTypeID parameterType,
        set_context setContext, get_setting getSetting)
        : Option{std::move(name), parameterType, OptionType::CONFIGURATION}, setContext{setContext},
          getSetting{getSetting} {}
};

struct ExtensionOption final : Option {
    common::Value defaultValue;

    ExtensionOption(std::string name, common::LogicalTypeID parameterType,
        common::Value defaultValue)
        : Option{std::move(name), parameterType, OptionType::EXTENSION},
          defaultValue{std::move(defaultValue)} {}
};

struct DBConfig {
    uint64_t bufferPoolSize;
    uint64_t maxNumThreads;
    bool enableCompression;
    bool readOnly;
    uint64_t maxDBSize;
    bool enableMultiWrites;
    bool autoCheckpoint;
    uint64_t checkpointThreshold;
    bool forceCheckpointOnClose;

    explicit DBConfig(const SystemConfig& systemConfig);

    static ConfigurationOption* getOptionByName(const std::string& optionName);
    KUZU_API static bool isDBPathInMemory(const std::string& dbPath);
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace common {

struct CSVOption {
    // TODO(Xiyang): Add newline character option and delimiter can be a string.
    char escapeChar;
    char delimiter;
    char quoteChar;
    bool hasHeader;
    uint64_t skipNum;

    CSVOption()
        : escapeChar{CopyConstants::DEFAULT_CSV_ESCAPE_CHAR},
          delimiter{CopyConstants::DEFAULT_CSV_DELIMITER},
          quoteChar{CopyConstants::DEFAULT_CSV_QUOTE_CHAR},
          hasHeader{CopyConstants::DEFAULT_CSV_HAS_HEADER},
          skipNum{CopyConstants::DEFAULT_CSV_SKIP_NUM} {}
    EXPLICIT_COPY_DEFAULT_MOVE(CSVOption);

    // TODO: COPY FROM and COPY TO should support transform special options, like '\'.
    std::string toCypher() const {
        std::string header = hasHeader ? "true" : "false";
        return stringFormat("(escape ='\\{}', delim ='{}', quote='\\{}', header={})", escapeChar,
            delimiter, quoteChar, header);
    }

private:
    CSVOption(const CSVOption& other)
        : escapeChar{other.escapeChar}, delimiter{other.delimiter}, quoteChar{other.quoteChar},
          hasHeader{other.hasHeader}, skipNum{other.skipNum} {}
};

struct CSVReaderConfig {
    CSVOption option;
    bool parallel;

    CSVReaderConfig() : option{}, parallel{CopyConstants::DEFAULT_CSV_PARALLEL} {}
    EXPLICIT_COPY_DEFAULT_MOVE(CSVReaderConfig);

    static CSVReaderConfig construct(const std::unordered_map<std::string, common::Value>& options);

private:
    CSVReaderConfig(const CSVReaderConfig& other)
        : option{other.option.copy()}, parallel{other.parallel} {}
};

} // namespace common
} // namespace kuzu

#include <numeric>
#include <utility>


namespace kuzu {
namespace common {

class Value;

//! A Vector represents values of the same data type.
//! The capacity of a ValueVector is either 1 (sequence) or DEFAULT_VECTOR_CAPACITY.
class KUZU_API ValueVector {
    friend class ListVector;
    friend class ListAuxiliaryBuffer;
    friend class StructVector;
    friend class StringVector;
    friend class ArrowColumnVector;

public:
    explicit ValueVector(LogicalType dataType, storage::MemoryManager* memoryManager = nullptr);
    explicit ValueVector(LogicalTypeID dataTypeID, storage::MemoryManager* memoryManager = nullptr)
        : ValueVector(LogicalType(dataTypeID), memoryManager) {
        KU_ASSERT(dataTypeID != LogicalTypeID::LIST);
    }

    ~ValueVector() = default;

    void setState(const std::shared_ptr<DataChunkState>& state_);

    void setAllNull() { nullMask.setAllNull(); }
    void setAllNonNull() { nullMask.setAllNonNull(); }
    // On return true, there are no null. On return false, there may or may not be nulls.
    bool hasNoNullsGuarantee() const { return nullMask.hasNoNullsGuarantee(); }
    void setNullRange(uint32_t startPos, uint32_t len, bool value) {
        nullMask.setNullFromRange(startPos, len, value);
    }
    const NullMask& getNullMask() const { return nullMask; }
    void setNull(uint32_t pos, bool isNull);
    uint8_t isNull(uint32_t pos) const { return nullMask.isNull(pos); }
    void setAsSingleNullEntry() {
        state->getSelVectorUnsafe().setSelSize(1);
        setNull(state->getSelVector()[0], true);
    }

    bool setNullFromBits(const uint64_t* srcNullEntries, uint64_t srcOffset, uint64_t dstOffset,
        uint64_t numBitsToCopy, bool invert = false);

    uint32_t getNumBytesPerValue() const { return numBytesPerValue; }

    // TODO(Guodong): Rename this to getValueRef
    template<typename T>
    const T& getValue(uint32_t pos) const {
        return ((T*)valueBuffer.get())[pos];
    }
    template<typename T>
    T& getValue(uint32_t pos) {
        return ((T*)valueBuffer.get())[pos];
    }
    template<typename T>
    void setValue(uint32_t pos, T val);
    // copyFromRowData assumes rowData is non-NULL.
    void copyFromRowData(uint32_t pos, const uint8_t* rowData);
    // copyToRowData assumes srcVectorData is non-NULL.
    void copyToRowData(uint32_t pos, uint8_t* rowData,
        InMemOverflowBuffer* rowOverflowBuffer) const;
    // copyFromVectorData assumes srcVectorData is non-NULL.
    void copyFromVectorData(uint8_t* dstData, const ValueVector* srcVector,
        const uint8_t* srcVectorData);
    void copyFromVectorData(uint64_t dstPos, const ValueVector* srcVector, uint64_t srcPos);
    void copyFromValue(uint64_t pos, const Value& value);

    std::unique_ptr<Value> getAsValue(uint64_t pos) const;

    uint8_t* getData() const { return valueBuffer.get(); }

    offset_t readNodeOffset(uint32_t pos) const {
        KU_ASSERT(dataType.getLogicalTypeID() == LogicalTypeID::INTERNAL_ID);
        return getValue<nodeID_t>(pos).offset;
    }

    void resetAuxiliaryBuffer();

    // If there is still non-null values after discarding, return true. Otherwise, return false.
    // For an unflat vector, its selection vector is also updated to the resultSelVector.
    static bool discardNull(ValueVector& vector);

    void serialize(Serializer& ser) const;
    static std::unique_ptr<ValueVector> deSerialize(Deserializer& deSer, storage::MemoryManager* mm,
        std::shared_ptr<DataChunkState> dataChunkState);

private:
    uint32_t getDataTypeSize(const LogicalType& type);
    void initializeValueBuffer();

public:
    LogicalType dataType;
    std::shared_ptr<DataChunkState> state;

private:
    std::unique_ptr<uint8_t[]> valueBuffer;
    NullMask nullMask;
    uint32_t numBytesPerValue;
    std::unique_ptr<AuxiliaryBuffer> auxiliaryBuffer;
};

class KUZU_API StringVector {
public:
    static inline InMemOverflowBuffer* getInMemOverflowBuffer(ValueVector* vector) {
        KU_ASSERT(vector->dataType.getPhysicalType() == PhysicalTypeID::STRING);
        return ku_dynamic_cast<AuxiliaryBuffer*, StringAuxiliaryBuffer*>(
            vector->auxiliaryBuffer.get())
            ->getOverflowBuffer();
    }

    static void addString(ValueVector* vector, uint32_t vectorPos, ku_string_t& srcStr);
    static void addString(ValueVector* vector, uint32_t vectorPos, const char* srcStr,
        uint64_t length);
    static void addString(ValueVector* vector, uint32_t vectorPos, const std::string& srcStr);
    // Add empty string with space reserved for the provided size
    // Returned value can be modified to set the string contents
    static ku_string_t& reserveString(ValueVector* vector, uint32_t vectorPos, uint64_t length);
    static void reserveString(ValueVector* vector, ku_string_t& dstStr, uint64_t length);
    static void addString(ValueVector* vector, ku_string_t& dstStr, ku_string_t& srcStr);
    static void addString(ValueVector* vector, ku_string_t& dstStr, const char* srcStr,
        uint64_t length);
    static void addString(kuzu::common::ValueVector* vector, ku_string_t& dstStr,
        const std::string& srcStr);
    static void copyToRowData(const ValueVector* vector, uint32_t pos, uint8_t* rowData,
        InMemOverflowBuffer* rowOverflowBuffer);
};

struct KUZU_API BlobVector {
    static void addBlob(ValueVector* vector, uint32_t pos, const char* data, uint32_t length) {
        StringVector::addString(vector, pos, data, length);
    }
    static void addBlob(ValueVector* vector, uint32_t pos, const uint8_t* data, uint64_t length) {
        StringVector::addString(vector, pos, reinterpret_cast<const char*>(data), length);
    }
};

// ListVector is used for both LIST and ARRAY physical type
class KUZU_API ListVector {
public:
    static const ListAuxiliaryBuffer& getAuxBuffer(const ValueVector& vector) {
        return vector.auxiliaryBuffer->constCast<ListAuxiliaryBuffer>();
    }
    static ListAuxiliaryBuffer& getAuxBufferUnsafe(const ValueVector& vector) {
        return vector.auxiliaryBuffer->cast<ListAuxiliaryBuffer>();
    }
    // If you call setDataVector during initialize, there must be a followed up
    // copyListEntryAndBufferMetaData at runtime.
    // TODO(Xiyang): try to merge setDataVector & copyListEntryAndBufferMetaData
    static void setDataVector(const ValueVector* vector, std::shared_ptr<ValueVector> dataVector) {
        KU_ASSERT(validateType(*vector));
        auto& listBuffer = getAuxBufferUnsafe(*vector);
        listBuffer.setDataVector(std::move(dataVector));
    }
    static void copyListEntryAndBufferMetaData(ValueVector& vector, const ValueVector& other);
    static ValueVector* getDataVector(const ValueVector* vector) {
        KU_ASSERT(validateType(*vector));
        return getAuxBuffer(*vector).getDataVector();
    }
    static std::shared_ptr<ValueVector> getSharedDataVector(const ValueVector* vector) {
        KU_ASSERT(validateType(*vector));
        return getAuxBuffer(*vector).getSharedDataVector();
    }
    static uint64_t getDataVectorSize(const ValueVector* vector) {
        KU_ASSERT(validateType(*vector));
        return getAuxBuffer(*vector).getSize();
    }
    static uint8_t* getListValues(const ValueVector* vector, const list_entry_t& listEntry) {
        KU_ASSERT(validateType(*vector));
        auto dataVector = getDataVector(vector);
        return dataVector->getData() + dataVector->getNumBytesPerValue() * listEntry.offset;
    }
    static uint8_t* getListValuesWithOffset(const ValueVector* vector,
        const list_entry_t& listEntry, offset_t elementOffsetInList) {
        KU_ASSERT(validateType(*vector));
        return getListValues(vector, listEntry) +
               elementOffsetInList * getDataVector(vector)->getNumBytesPerValue();
    }
    static list_entry_t addList(ValueVector* vector, uint64_t listSize) {
        KU_ASSERT(validateType(*vector));
        return getAuxBufferUnsafe(*vector).addList(listSize);
    }
    static void resizeDataVector(ValueVector* vector, uint64_t numValues) {
        KU_ASSERT(validateType(*vector));
        getAuxBufferUnsafe(*vector).resize(numValues);
    }

    static void copyFromRowData(ValueVector* vector, uint32_t pos, const uint8_t* rowData);
    static void copyToRowData(const ValueVector* vector, uint32_t pos, uint8_t* rowData,
        InMemOverflowBuffer* rowOverflowBuffer);
    static void copyFromVectorData(ValueVector* dstVector, uint8_t* dstData,
        const ValueVector* srcVector, const uint8_t* srcData);
    static void appendDataVector(ValueVector* dstVector, ValueVector* srcDataVector,
        uint64_t numValuesToAppend);
    static void sliceDataVector(ValueVector* vectorToSlice, uint64_t offset, uint64_t numValues);

private:
    static bool validateType(const ValueVector& vector) {
        switch (vector.dataType.getPhysicalType()) {
        case PhysicalTypeID::LIST:
        case PhysicalTypeID::ARRAY:
            return true;
        default:
            return false;
        }
    }
};

class StructVector {
public:
    static inline const std::vector<std::shared_ptr<ValueVector>>& getFieldVectors(
        const ValueVector* vector) {
        return ku_dynamic_cast<AuxiliaryBuffer*, StructAuxiliaryBuffer*>(
            vector->auxiliaryBuffer.get())
            ->getFieldVectors();
    }

    static inline std::shared_ptr<ValueVector> getFieldVector(const ValueVector* vector,
        struct_field_idx_t idx) {
        return ku_dynamic_cast<AuxiliaryBuffer*, StructAuxiliaryBuffer*>(
            vector->auxiliaryBuffer.get())
            ->getFieldVectors()[idx];
    }

    static inline void referenceVector(ValueVector* vector, struct_field_idx_t idx,
        std::shared_ptr<ValueVector> vectorToReference) {
        ku_dynamic_cast<AuxiliaryBuffer*, StructAuxiliaryBuffer*>(vector->auxiliaryBuffer.get())
            ->referenceChildVector(idx, std::move(vectorToReference));
    }

    static inline void initializeEntries(ValueVector* vector) {
        std::iota(reinterpret_cast<int64_t*>(vector->getData()),
            reinterpret_cast<int64_t*>(
                vector->getData() + vector->getNumBytesPerValue() * DEFAULT_VECTOR_CAPACITY),
            0);
    }

    static void copyFromRowData(ValueVector* vector, uint32_t pos, const uint8_t* rowData);
    static void copyToRowData(const ValueVector* vector, uint32_t pos, uint8_t* rowData,
        InMemOverflowBuffer* rowOverflowBuffer);
    static void copyFromVectorData(ValueVector* dstVector, const uint8_t* dstData,
        const ValueVector* srcVector, const uint8_t* srcData);
};

class UnionVector {
public:
    static inline ValueVector* getTagVector(const ValueVector* vector) {
        KU_ASSERT(vector->dataType.getLogicalTypeID() == LogicalTypeID::UNION);
        return StructVector::getFieldVector(vector, UnionType::TAG_FIELD_IDX).get();
    }

    static inline ValueVector* getValVector(const ValueVector* vector, union_field_idx_t fieldIdx) {
        KU_ASSERT(vector->dataType.getLogicalTypeID() == LogicalTypeID::UNION);
        return StructVector::getFieldVector(vector, UnionType::getInternalFieldIdx(fieldIdx)).get();
    }

    static inline void referenceVector(ValueVector* vector, union_field_idx_t fieldIdx,
        std::shared_ptr<ValueVector> vectorToReference) {
        StructVector::referenceVector(vector, UnionType::getInternalFieldIdx(fieldIdx),
            std::move(vectorToReference));
    }

    static inline void setTagField(ValueVector* vector, union_field_idx_t tag) {
        KU_ASSERT(vector->dataType.getLogicalTypeID() == LogicalTypeID::UNION);
        for (auto i = 0u; i < vector->state->getSelVector().getSelSize(); i++) {
            vector->setValue<struct_field_idx_t>(vector->state->getSelVector()[i], tag);
        }
    }
};

class MapVector {
public:
    static inline ValueVector* getKeyVector(const ValueVector* vector) {
        return StructVector::getFieldVector(ListVector::getDataVector(vector), 0 /* keyVectorPos */)
            .get();
    }

    static inline ValueVector* getValueVector(const ValueVector* vector) {
        return StructVector::getFieldVector(ListVector::getDataVector(vector), 1 /* valVectorPos */)
            .get();
    }

    static inline uint8_t* getMapKeys(const ValueVector* vector, const list_entry_t& listEntry) {
        auto keyVector = getKeyVector(vector);
        return keyVector->getData() + keyVector->getNumBytesPerValue() * listEntry.offset;
    }

    static inline uint8_t* getMapValues(const ValueVector* vector, const list_entry_t& listEntry) {
        auto valueVector = getValueVector(vector);
        return valueVector->getData() + valueVector->getNumBytesPerValue() * listEntry.offset;
    }
};

struct RdfVariantVector {
    static void addString(ValueVector* vector, sel_t pos, ku_string_t str);
    static void addString(ValueVector* vector, sel_t pos, const char* str, uint32_t length);

    template<typename T>
    static void add(ValueVector* vector, sel_t pos, T val);
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace catalog {

class FunctionCatalogEntry : public CatalogEntry {
public:
    //===--------------------------------------------------------------------===//
    // constructors
    //===--------------------------------------------------------------------===//
    FunctionCatalogEntry() = default;
    FunctionCatalogEntry(CatalogEntryType entryType, std::string name,
        function::function_set functionSet);

    //===--------------------------------------------------------------------===//
    // getters & setters
    //===--------------------------------------------------------------------===//
    const function::function_set& getFunctionSet() const { return functionSet; }

    //===--------------------------------------------------------------------===//
    // serialization & deserialization
    //===--------------------------------------------------------------------===//
    // We always register functions while initializing the catalog, so we don't have to
    // serialize functions.
    void serialize(common::Serializer& /*serializer*/) const override { return; }

protected:
    function::function_set functionSet;
};

} // namespace catalog
} // namespace kuzu

#include <functional>


namespace kuzu {
namespace evaluator {
class ExpressionEvaluator;
} // namespace evaluator

namespace transaction {
class Transaction;
} // namespace transaction

namespace storage {

class Column;
class NullChunkData;
// TODO(Guodong): Ideally ColumnChunkMetadata should implement its own ser/deSer functions so it can
// save a bit of space on disk. But the size is small now, I'm not motivated for the change, just
// note here still.
struct ColumnChunkMetadata {
    common::page_idx_t pageIdx;
    common::page_idx_t numPages;
    uint64_t numValues;
    CompressionMetadata compMeta;

    // TODO(Guodong): Delete copy constructor.
    ColumnChunkMetadata()
        : pageIdx{common::INVALID_PAGE_IDX}, numPages{0}, numValues{0},
          compMeta(StorageValue(), StorageValue(), CompressionType::CONSTANT) {}
    ColumnChunkMetadata(common::page_idx_t pageIdx, common::page_idx_t numPages,
        uint64_t numNodesInChunk, const CompressionMetadata& compMeta)
        : pageIdx(pageIdx), numPages(numPages), numValues(numNodesInChunk), compMeta(compMeta) {}
};

// TODO(bmwinger): Hide access to variables.
struct ChunkState {
    Column* column;
    ColumnChunkMetadata metadata;
    uint64_t numValuesPerPage = UINT64_MAX;
    std::unique_ptr<ChunkState> nullState;
    // Used for struct/list/string columns.
    std::vector<ChunkState> childrenStates;

    explicit ChunkState(bool hasNull = true) : column{nullptr} {
        if (hasNull) {
            nullState = std::make_unique<ChunkState>(false /*hasNull*/);
        }
    }
    ChunkState(ColumnChunkMetadata metadata, uint64_t numValuesPerPage)
        : column{nullptr}, metadata{std::move(metadata)}, numValuesPerPage{numValuesPerPage} {
        nullState = std::make_unique<ChunkState>(false /*hasNull*/);
    }

    ChunkState& getChildState(common::idx_t childIdx) {
        KU_ASSERT(childIdx < childrenStates.size());
        return childrenStates[childIdx];
    }
    const ChunkState& getChildState(common::idx_t childIdx) const {
        KU_ASSERT(childIdx < childrenStates.size());
        return childrenStates[childIdx];
    }

    void resetState() {
        numValuesPerPage = UINT64_MAX;
        if (nullState) {
            nullState->resetState();
        }
        for (auto& childState : childrenStates) {
            childState.resetState();
        }
    }
};

class BMFileHandle;
// Base data segment covers all fixed-sized data types.
class ColumnChunkData {
public:
    friend struct ColumnChunkFactory;

    ColumnChunkData(common::LogicalType dataType, uint64_t capacity, bool enableCompression,
        ResidencyState residencyState, bool hasNullData);
    ColumnChunkData(common::LogicalType dataType, bool enableCompression,
        const ColumnChunkMetadata& metadata, bool hasNullData);
    virtual ~ColumnChunkData() = default;

    template<typename T>
    T getValue(common::offset_t pos) const {
        KU_ASSERT(pos < numValues);
        KU_ASSERT(residencyState != ResidencyState::ON_DISK);
        return reinterpret_cast<T*>(buffer.get())[pos];
    }
    template<typename T>
    void setValue(T val, common::offset_t pos) {
        KU_ASSERT(pos < capacity);
        KU_ASSERT(residencyState != ResidencyState::ON_DISK);
        reinterpret_cast<T*>(buffer.get())[pos] = val;
        if (pos >= numValues) {
            numValues = pos + 1;
        }
    }

    bool isNull(common::offset_t pos) const;
    void setNullData(std::unique_ptr<NullChunkData> nullData_) { nullData = std::move(nullData_); }
    bool hasNullData() const { return nullData != nullptr; }
    NullChunkData* getNullData() { return nullData.get(); }
    const NullChunkData& getNullData() const { return *nullData; }
    std::optional<common::NullMask> getNullMask() const;
    std::unique_ptr<NullChunkData> moveNullData() { return std::move(nullData); }

    common::LogicalType& getDataType() { return dataType; }
    const common::LogicalType& getDataType() const { return dataType; }
    ResidencyState getResidencyState() const { return residencyState; }
    bool isCompressionEnabled() const { return enableCompression; }
    ColumnChunkMetadata& getMetadata() {
        KU_ASSERT(residencyState == ResidencyState::ON_DISK);
        return metadata;
    }
    const ColumnChunkMetadata& getMetadata() const {
        KU_ASSERT(residencyState == ResidencyState::ON_DISK);
        return metadata;
    }
    void setMetadata(const ColumnChunkMetadata& metadata_) {
        KU_ASSERT(residencyState == ResidencyState::ON_DISK);
        metadata = metadata_;
    }

    // Only have side effects on in-memory or temporary chunks.
    virtual void resetToAllNull();
    virtual void resetToEmpty();

    // Note that the startPageIdx is not known, so it will always be common::INVALID_PAGE_IDX
    virtual ColumnChunkMetadata getMetadataToFlush() const;

    virtual void append(common::ValueVector* vector, const common::SelectionVector& selVector);
    virtual void append(ColumnChunkData* other, common::offset_t startPosInOtherChunk,
        uint32_t numValuesToAppend);

    virtual void flush(BMFileHandle& dataFH);

    ColumnChunkMetadata flushBuffer(BMFileHandle* dataFH, common::page_idx_t startPageIdx,
        const ColumnChunkMetadata& metadata) const;

    static common::page_idx_t getNumPagesForBytes(uint64_t numBytes) {
        return (numBytes + common::BufferPoolConstants::PAGE_4KB_SIZE - 1) /
               common::BufferPoolConstants::PAGE_4KB_SIZE;
    }

    uint64_t getNumBytesPerValue() const { return numBytesPerValue; }
    uint8_t* getData() const { return buffer.get(); }

    virtual void initializeScanState(ChunkState& state) const;
    virtual void scan(common::ValueVector& output, common::offset_t offset, common::length_t length,
        common::sel_t posInOutputVector = 0) const;
    virtual void lookup(common::offset_t offsetInChunk, common::ValueVector& output,
        common::sel_t posInOutputVector) const;

    // TODO(Guodong): In general, this is not a good interface. Instead of passing in
    // `offsetInVector`, we should flatten the vector to pos at `offsetInVector`.
    virtual void write(const common::ValueVector* vector, common::offset_t offsetInVector,
        common::offset_t offsetInChunk);
    virtual void write(ColumnChunkData* chunk, ColumnChunkData* offsetsInChunk,
        common::RelMultiplicity multiplicity);
    virtual void write(ColumnChunkData* srcChunk, common::offset_t srcOffsetInChunk,
        common::offset_t dstOffsetInChunk, common::offset_t numValuesToCopy);
    // TODO(Guodong): Used in `applyDeletionsToChunk`. Should unify with `write`.
    virtual void copy(ColumnChunkData* srcChunk, common::offset_t srcOffsetInChunk,
        common::offset_t dstOffsetInChunk, common::offset_t numValuesToCopy);

    virtual void setToInMemory();
    // numValues must be at least the number of values the ColumnChunk was first initialized
    // with
    virtual void resize(uint64_t newCapacity);

    void populateWithDefaultVal(evaluator::ExpressionEvaluator& defaultEvaluator,
        uint64_t& numValues_);
    virtual void finalize() {
        KU_ASSERT(residencyState != ResidencyState::ON_DISK);
        // DO NOTHING.
    }

    uint64_t getCapacity() const { return capacity; }
    virtual uint64_t getNumValues() const { return numValues; }
    // TODO(Guodong): Alternatively, we can let `getNumValues` read from metadata when ON_DISK.
    virtual void resetNumValuesFromMetadata();
    virtual void setNumValues(uint64_t numValues_);
    virtual void syncNumValues() {}
    virtual bool numValuesSanityCheck() const;

    virtual bool sanityCheck() const;

    virtual uint64_t getEstimatedMemoryUsage() const;

    virtual void serialize(common::Serializer& serializer) const;
    static std::unique_ptr<ColumnChunkData> deserialize(common::Deserializer& deSer);

    template<typename TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<ColumnChunkData&, TARGET&>(*this);
    }
    template<typename TARGET>
    const TARGET& cast() const {
        return common::ku_dynamic_cast<const ColumnChunkData&, const TARGET&>(*this);
    }

protected:
    // Initializes the data buffer and functions. They are (and should be) only called in
    // constructor.
    void initializeBuffer();
    void initializeFunction(bool enableCompression);

    // Note: This function is not setting child/null chunk data recursively.
    void setToOnDisk(const ColumnChunkMetadata& metadata);

    virtual void copyVectorToBuffer(common::ValueVector* vector, common::offset_t startPosInChunk,
        const common::SelectionVector& selVector);

private:
    uint64_t getBufferSize(uint64_t capacity_) const;

protected:
    using flush_buffer_func_t = std::function<ColumnChunkMetadata(const uint8_t*, uint64_t,
        BMFileHandle*, common::page_idx_t, const ColumnChunkMetadata&)>;
    using get_metadata_func_t = std::function<ColumnChunkMetadata(const uint8_t*, uint64_t,
        uint64_t, uint64_t, StorageValue, StorageValue)>;
    using get_min_max_func_t =
        std::function<std::pair<StorageValue, StorageValue>(const uint8_t*, uint64_t)>;

    ResidencyState residencyState;
    common::LogicalType dataType;
    bool enableCompression;
    uint32_t numBytesPerValue;
    uint64_t bufferSize;
    uint64_t capacity;
    std::unique_ptr<uint8_t[]> buffer;
    std::unique_ptr<NullChunkData> nullData;
    uint64_t numValues;
    flush_buffer_func_t flushBufferFunction;
    get_metadata_func_t getMetadataFunction;

    // On-disk metadata for column chunk.
    ColumnChunkMetadata metadata;
};

template<>
inline void ColumnChunkData::setValue(bool val, common::offset_t pos) {
    // Buffer is rounded up to the nearest 8 bytes so that this cast is safe
    common::NullMask::setNull(reinterpret_cast<uint64_t*>(buffer.get()), pos, val);
}

template<>
inline bool ColumnChunkData::getValue(common::offset_t pos) const {
    // Buffer is rounded up to the nearest 8 bytes so that this cast is safe
    return common::NullMask::isNull(reinterpret_cast<uint64_t*>(buffer.get()), pos);
}

// Stored as bitpacked booleans in-memory and on-disk
class BoolChunkData : public ColumnChunkData {
public:
    BoolChunkData(uint64_t capacity, bool enableCompression, ResidencyState type, bool hasNullChunk)
        : ColumnChunkData(common::LogicalType::BOOL(), capacity,
              // Booleans are always bitpacked, but this can also enable constant compression
              enableCompression, type, hasNullChunk) {}
    BoolChunkData(bool enableCompression, const ColumnChunkMetadata& metadata, bool hasNullData)
        : ColumnChunkData{common::LogicalType::BOOL(), enableCompression, metadata, hasNullData} {}

    void append(common::ValueVector* vector, const common::SelectionVector& sel) final;
    void append(ColumnChunkData* other, common::offset_t startPosInOtherChunk,
        uint32_t numValuesToAppend) override;

    void scan(common::ValueVector& output, common::offset_t offset, common::length_t length,
        common::sel_t posInOutputVector = 0) const override;
    void lookup(common::offset_t offsetInChunk, common::ValueVector& output,
        common::sel_t posInOutputVector) const override;

    void write(const common::ValueVector* vector, common::offset_t offsetInVector,
        common::offset_t offsetInChunk) override;
    void write(ColumnChunkData* chunk, ColumnChunkData* dstOffsets,
        common::RelMultiplicity multiplicity) final;
    void write(ColumnChunkData* srcChunk, common::offset_t srcOffsetInChunk,
        common::offset_t dstOffsetInChunk, common::offset_t numValuesToCopy) override;
};

class NullChunkData final : public BoolChunkData {
public:
    NullChunkData(uint64_t capacity, bool enableCompression, ResidencyState type)
        : BoolChunkData(capacity, enableCompression, type, false /*hasNullData*/),
          mayHaveNullValue{false} {}
    NullChunkData(bool enableCompression, const ColumnChunkMetadata& metadata)
        : BoolChunkData{enableCompression, metadata, false /*hasNullData*/},
          mayHaveNullValue{false} {}

    // Maybe this should be combined with BoolChunkData if the only difference is these functions?
    bool isNull(common::offset_t pos) const { return getValue<bool>(pos); }
    void setNull(common::offset_t pos, bool isNull);

    bool mayHaveNull() const { return mayHaveNullValue; }

    void resetToEmpty() override {
        resetToNoNull();
        numValues = 0;
    }
    void resetToNoNull() {
        memset(buffer.get(), 0 /* non null */, bufferSize);
        mayHaveNullValue = false;
    }
    void resetToAllNull() override {
        memset(buffer.get(), 0xFF /* null */, bufferSize);
        mayHaveNullValue = true;
    }

    void copyFromBuffer(uint64_t* srcBuffer, uint64_t srcOffset, uint64_t dstOffset,
        uint64_t numBits, bool invert = false) {
        if (common::NullMask::copyNullMask(srcBuffer, srcOffset,
                reinterpret_cast<uint64_t*>(buffer.get()), dstOffset, numBits, invert)) {
            mayHaveNullValue = true;
        }
    }

    // NullChunkData::scan updates the null mask of output vector
    void scan(common::ValueVector& output, common::offset_t offset, common::length_t length,
        common::sel_t posInOutputVector = 0) const override;

    void append(ColumnChunkData* other, common::offset_t startPosInOtherChunk,
        uint32_t numValuesToAppend) override;

    void write(const common::ValueVector* vector, common::offset_t offsetInVector,
        common::offset_t offsetInChunk) override;
    void write(ColumnChunkData* srcChunk, common::offset_t srcOffsetInChunk,
        common::offset_t dstOffsetInChunk, common::offset_t numValuesToCopy) override;

    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<NullChunkData> deserialize(common::Deserializer& deSer);

    common::NullMask getNullMask() const {
        return common::NullMask(std::span(reinterpret_cast<uint64_t*>(buffer.get()), capacity / 64),
            mayHaveNullValue);
    }

protected:
    bool mayHaveNullValue;
};

class InternalIDChunkData final : public ColumnChunkData {
public:
    // TODO(Guodong): Should make InternalIDChunkData has no NULL.
    // Physically, we only materialize offset of INTERNAL_ID, which is same as UINT64,
    InternalIDChunkData(uint64_t capacity, bool enableCompression, ResidencyState residencyState)
        : ColumnChunkData(common::LogicalType::INTERNAL_ID(), capacity, enableCompression,
              residencyState, false /*hasNullData*/),
          commonTableID{common::INVALID_TABLE_ID} {}
    InternalIDChunkData(bool enableCompression, const ColumnChunkMetadata& metadata)
        : ColumnChunkData{common::LogicalType::INTERNAL_ID(), enableCompression, metadata,
              false /*hasNullData*/},
          commonTableID{common::INVALID_TABLE_ID} {}

    void append(common::ValueVector* vector, const common::SelectionVector& selVector) override;

    void copyVectorToBuffer(common::ValueVector* vector, common::offset_t startPosInChunk,
        const common::SelectionVector& selVector) override;

    void copyInt64VectorToBuffer(common::ValueVector* vector, common::offset_t startPosInChunk,
        const common::SelectionVector& selVector) const;

    void scan(common::ValueVector& output, common::offset_t offset, common::length_t length,
        common::sel_t posInOutputVector = 0) const override;
    void lookup(common::offset_t offsetInChunk, common::ValueVector& output,
        common::sel_t posInOutputVector) const override;

    void write(const common::ValueVector* vector, common::offset_t offsetInVector,
        common::offset_t offsetInChunk) override;

    void append(ColumnChunkData* other, common::offset_t startPosInOtherChunk,
        uint32_t numValuesToAppend) override;

    void setTableID(common::table_id_t tableID) { commonTableID = tableID; }
    common::table_id_t getTableID() const { return commonTableID; }

    common::offset_t operator[](common::offset_t pos) const {
        return getValue<common::offset_t>(pos);
    }
    common::offset_t& operator[](common::offset_t pos) {
        return reinterpret_cast<common::offset_t*>(buffer.get())[pos];
    }

private:
    common::table_id_t commonTableID;
};

struct ColumnChunkFactory {
    static std::unique_ptr<ColumnChunkData> createColumnChunkData(common::LogicalType dataType,
        bool enableCompression, uint64_t capacity, ResidencyState residencyState,
        bool hasNullData = true);
    static std::unique_ptr<ColumnChunkData> createColumnChunkData(common::LogicalType dataType,
        bool enableCompression, ColumnChunkMetadata& metadata, bool hasNullData);

    static std::unique_ptr<ColumnChunkData> createNullChunkData(bool enableCompression,
        uint64_t capacity, ResidencyState type) {
        return std::make_unique<NullChunkData>(capacity, enableCompression, type);
    }
};

} // namespace storage
} // namespace kuzu

#include <vector>


namespace kuzu {
namespace main {
class ClientContext;
}

namespace function {

struct TableFuncBindInput {
    std::vector<common::Value> inputs;

    TableFuncBindInput() = default;
    EXPLICIT_COPY_DEFAULT_MOVE(TableFuncBindInput);
    virtual ~TableFuncBindInput() = default;

    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const TableFuncBindInput*, const TARGET*>(this);
    }

protected:
    TableFuncBindInput(const TableFuncBindInput& other) : inputs{other.inputs} {}
};

struct ScanTableFuncBindInput final : public TableFuncBindInput {
    common::ReaderConfig config;
    std::vector<std::string> expectedColumnNames;
    std::vector<common::LogicalType> expectedColumnTypes;
    main::ClientContext* context;

    explicit ScanTableFuncBindInput(common::ReaderConfig config) : config{std::move(config)} {};
    ScanTableFuncBindInput(common::ReaderConfig config,
        std::vector<std::string> expectedColumnNames,
        std::vector<common::LogicalType> expectedColumnTypes, main::ClientContext* context)
        : TableFuncBindInput{}, config{std::move(config)},
          expectedColumnNames{std::move(expectedColumnNames)},
          expectedColumnTypes{std::move(expectedColumnTypes)}, context{context} {
        inputs.push_back(common::Value::createValue(this->config.filePaths[0]));
    }
    EXPLICIT_COPY_DEFAULT_MOVE(ScanTableFuncBindInput);

private:
    ScanTableFuncBindInput(const ScanTableFuncBindInput& other)
        : TableFuncBindInput{other}, config{other.config.copy()},
          expectedColumnNames{other.expectedColumnNames},
          expectedColumnTypes{common::LogicalType::copy(other.expectedColumnTypes)},
          context{other.context} {}
};

} // namespace function
} // namespace kuzu

#include <string>


namespace kuzu {
namespace main {

/**
 * @brief QueryResult stores the result of a query execution.
 */
class QueryResult {
    friend class Connection;
    friend class ClientContext;
    class QueryResultIterator {
    private:
        QueryResult* currentResult;

    public:
        QueryResultIterator() = default;

        explicit QueryResultIterator(QueryResult* startResult) : currentResult(startResult) {}

        void operator++() {
            if (currentResult) {
                currentResult = currentResult->nextQueryResult.get();
            }
        }

        bool isEnd() const { return currentResult == nullptr; }

        bool hasNextQueryResult() const { return currentResult->nextQueryResult != nullptr; }

        QueryResult* getCurrentResult() const { return currentResult; }
    };

public:
    /**
     * @brief Used to create a QueryResult object for the failing query.
     */
    KUZU_API QueryResult();

    explicit QueryResult(const PreparedSummary& preparedSummary);
    /**
     * @brief Deconstructs the QueryResult object.
     */
    KUZU_API ~QueryResult();
    /**
     * @return query is executed successfully or not.
     */
    KUZU_API bool isSuccess() const;
    /**
     * @return error message of the query execution if the query fails.
     */
    KUZU_API std::string getErrorMessage() const;
    /**
     * @return number of columns in query result.
     */
    KUZU_API size_t getNumColumns() const;
    /**
     * @return name of each column in query result.
     */
    KUZU_API std::vector<std::string> getColumnNames() const;
    /**
     * @return dataType of each column in query result.
     */
    KUZU_API std::vector<common::LogicalType> getColumnDataTypes() const;
    /**
     * @return num of tuples in query result.
     */
    KUZU_API uint64_t getNumTuples() const;
    /**
     * @return query summary which stores the execution time, compiling time, plan and query
     * options.
     */
    KUZU_API QuerySummary* getQuerySummary() const;
    /**
     * @return whether there are more tuples to read.
     */
    KUZU_API bool hasNext() const;
    /**
     * @return whether there are more query results to read.
     */
    KUZU_API bool hasNextQueryResult() const;
    /**
     * @return get next query result to read (for multiple query statements).
     */
    KUZU_API QueryResult* getNextQueryResult();

    std::unique_ptr<QueryResult> nextQueryResult;
    /**
     * @return next flat tuple in the query result.
     */
    KUZU_API std::shared_ptr<processor::FlatTuple> getNext();
    /**
     * @return string of first query result.
     */
    KUZU_API std::string toString();

    /**
     * @brief Resets the result tuple iterator.
     */
    KUZU_API void resetIterator();

    processor::FactorizedTable* getTable() { return factorizedTable.get(); }

    /**
     * @brief Returns the arrow schema of the query result.
     * @return datatypes of the columns as an arrow schema
     *
     * It is the caller's responsibility to call the release function to release the underlying data
     * If converting to another arrow type, this this is usually handled automatically.
     */
    KUZU_API std::unique_ptr<ArrowSchema> getArrowSchema() const;

    /**
     * @brief Returns the next chunk of the query result as an arrow array.
     * @param chunkSize number of tuples to return in the chunk.
     * @return An arrow array representation of the next chunkSize tuples of the query result.
     *
     * The ArrowArray internally stores an arrow struct with fields for each of the columns.
     * This can be converted to a RecordBatch with arrow's ImportRecordBatch function
     *
     * It is the caller's responsibility to call the release function to release the underlying data
     * If converting to another arrow type, this this is usually handled automatically.
     */
    KUZU_API std::unique_ptr<ArrowArray> getNextArrowChunk(int64_t chunkSize);

private:
    void initResultTableAndIterator(std::shared_ptr<processor::FactorizedTable> factorizedTable_,
        const std::vector<std::shared_ptr<binder::Expression>>& columns);
    void validateQuerySucceed() const;

private:
    // execution status
    bool success = true;
    std::string errMsg;

    // header information
    std::vector<std::string> columnNames;
    std::vector<common::LogicalType> columnDataTypes;
    // data
    std::shared_ptr<processor::FactorizedTable> factorizedTable;
    std::unique_ptr<processor::FlatTupleIterator> iterator;
    std::shared_ptr<processor::FlatTuple> tuple;

    // execution statistics
    std::unique_ptr<QuerySummary> querySummary;

    // query iterator
    QueryResultIterator queryResultIterator;
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace common {
enum class RelMultiplicity : uint8_t;
}
namespace binder {
struct BoundExtraCreateCatalogEntryInfo {
    virtual ~BoundExtraCreateCatalogEntryInfo() = default;

    template<class TARGET>
    const TARGET* constPtrCast() const {
        return common::ku_dynamic_cast<const BoundExtraCreateCatalogEntryInfo*, const TARGET*>(
            this);
    }

    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<BoundExtraCreateCatalogEntryInfo*, TARGET*>(this);
    }

    virtual void serialize(common::Serializer& serializer) const = 0;

    virtual inline std::unique_ptr<BoundExtraCreateCatalogEntryInfo> copy() const = 0;
};

struct BoundCreateTableInfo {
    common::TableType type;
    std::string tableName;
    common::ConflictAction onConflict;
    bool hasParent = false;
    std::unique_ptr<BoundExtraCreateCatalogEntryInfo> extraInfo;

    BoundCreateTableInfo() = default;
    BoundCreateTableInfo(common::TableType type, std::string tableName,
        common::ConflictAction onConflict,
        std::unique_ptr<BoundExtraCreateCatalogEntryInfo> extraInfo)
        : type{type}, tableName{std::move(tableName)}, onConflict{onConflict},
          extraInfo{std::move(extraInfo)} {}
    EXPLICIT_COPY_DEFAULT_MOVE(BoundCreateTableInfo);

    void serialize(common::Serializer& serializer) const;
    static BoundCreateTableInfo deserialize(common::Deserializer& deserializer);

private:
    BoundCreateTableInfo(const BoundCreateTableInfo& other)
        : type{other.type}, tableName{other.tableName}, onConflict{other.onConflict},
          hasParent{other.hasParent}, extraInfo{other.extraInfo->copy()} {}
};

struct PropertyInfo {
    std::string name;
    common::LogicalType type;
    std::unique_ptr<parser::ParsedExpression> defaultValue;

    PropertyInfo() = default;
    PropertyInfo(std::string name, common::LogicalType type)
        : PropertyInfo{name, std::move(type),
              std::make_unique<parser::ParsedLiteralExpression>(common::Value::createNullValue(),
                  "NULL")} {}

    PropertyInfo(std::string name, common::LogicalType type,
        std::unique_ptr<parser::ParsedExpression> defaultValue)
        : name{std::move(name)}, type{std::move(type)}, defaultValue{std::move(defaultValue)} {}
    EXPLICIT_COPY_DEFAULT_MOVE(PropertyInfo);

    void serialize(common::Serializer& serializer) const;
    static PropertyInfo deserialize(common::Deserializer& deserializer);

private:
    PropertyInfo(const PropertyInfo& other)
        : name{other.name}, type{other.type.copy()}, defaultValue{other.defaultValue->copy()} {}
};

struct KUZU_API BoundExtraCreateTableInfo : public BoundExtraCreateCatalogEntryInfo {
    std::vector<PropertyInfo> propertyInfos;

    explicit BoundExtraCreateTableInfo(std::vector<PropertyInfo> propertyInfos)
        : propertyInfos{std::move(propertyInfos)} {}

    BoundExtraCreateTableInfo(const BoundExtraCreateTableInfo& other)
        : BoundExtraCreateTableInfo{copyVector(other.propertyInfos)} {}
    BoundExtraCreateTableInfo& operator=(const BoundExtraCreateTableInfo&) = delete;

    std::unique_ptr<BoundExtraCreateCatalogEntryInfo> copy() const override {
        return std::make_unique<BoundExtraCreateTableInfo>(*this);
    }

    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<BoundExtraCreateTableInfo> deserialize(
        common::Deserializer& deserializer, common::TableType type);
};

struct BoundExtraCreateNodeTableInfo final : public BoundExtraCreateTableInfo {
    common::property_id_t primaryKeyIdx;

    BoundExtraCreateNodeTableInfo(common::property_id_t primaryKeyIdx,
        std::vector<PropertyInfo> propertyInfos)
        : BoundExtraCreateTableInfo{std::move(propertyInfos)}, primaryKeyIdx{primaryKeyIdx} {}
    BoundExtraCreateNodeTableInfo(const BoundExtraCreateNodeTableInfo& other)
        : BoundExtraCreateTableInfo{copyVector(other.propertyInfos)},
          primaryKeyIdx{other.primaryKeyIdx} {}

    std::unique_ptr<BoundExtraCreateCatalogEntryInfo> copy() const override {
        return std::make_unique<BoundExtraCreateNodeTableInfo>(*this);
    }

    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<BoundExtraCreateNodeTableInfo> deserialize(
        common::Deserializer& deserializer);
};

struct BoundExtraCreateRelTableInfo final : public BoundExtraCreateTableInfo {
    common::RelMultiplicity srcMultiplicity;
    common::RelMultiplicity dstMultiplicity;
    common::table_id_t srcTableID;
    common::table_id_t dstTableID;

    BoundExtraCreateRelTableInfo(common::table_id_t srcTableID, common::table_id_t dstTableID,
        std::vector<PropertyInfo> propertyInfos)
        : BoundExtraCreateRelTableInfo{common::RelMultiplicity::MANY, common::RelMultiplicity::MANY,
              srcTableID, dstTableID, std::move(propertyInfos)} {}
    BoundExtraCreateRelTableInfo(common::RelMultiplicity srcMultiplicity,
        common::RelMultiplicity dstMultiplicity, common::table_id_t srcTableID,
        common::table_id_t dstTableID, std::vector<PropertyInfo> propertyInfos)
        : BoundExtraCreateTableInfo{std::move(propertyInfos)}, srcMultiplicity{srcMultiplicity},
          dstMultiplicity{dstMultiplicity}, srcTableID{srcTableID}, dstTableID{dstTableID} {}
    BoundExtraCreateRelTableInfo(const BoundExtraCreateRelTableInfo& other)
        : BoundExtraCreateTableInfo{copyVector(other.propertyInfos)},
          srcMultiplicity{other.srcMultiplicity}, dstMultiplicity{other.dstMultiplicity},
          srcTableID{other.srcTableID}, dstTableID{other.dstTableID} {}

    std::unique_ptr<BoundExtraCreateCatalogEntryInfo> copy() const override {
        return std::make_unique<BoundExtraCreateRelTableInfo>(*this);
    }

    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<BoundExtraCreateRelTableInfo> deserialize(
        common::Deserializer& deserializer);
};

struct BoundExtraCreateRelTableGroupInfo final : public BoundExtraCreateCatalogEntryInfo {
    std::vector<BoundCreateTableInfo> infos;

    explicit BoundExtraCreateRelTableGroupInfo(std::vector<BoundCreateTableInfo> infos)
        : infos{std::move(infos)} {}
    BoundExtraCreateRelTableGroupInfo(const BoundExtraCreateRelTableGroupInfo& other)
        : infos{copyVector(other.infos)} {}

    inline std::unique_ptr<BoundExtraCreateCatalogEntryInfo> copy() const override {
        return std::make_unique<BoundExtraCreateRelTableGroupInfo>(*this);
    }

    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<BoundExtraCreateRelTableGroupInfo> deserialize(
        common::Deserializer& deserializer);
};

struct BoundExtraCreateRdfGraphInfo final : public BoundExtraCreateCatalogEntryInfo {
    BoundCreateTableInfo resourceInfo;
    BoundCreateTableInfo literalInfo;
    BoundCreateTableInfo resourceTripleInfo;
    BoundCreateTableInfo literalTripleInfo;

    BoundExtraCreateRdfGraphInfo(BoundCreateTableInfo resourceInfo,
        BoundCreateTableInfo literalInfo, BoundCreateTableInfo resourceTripleInfo,
        BoundCreateTableInfo literalTripleInfo)
        : resourceInfo{std::move(resourceInfo)}, literalInfo{std::move(literalInfo)},
          resourceTripleInfo{std::move(resourceTripleInfo)},
          literalTripleInfo{std::move(literalTripleInfo)} {}
    BoundExtraCreateRdfGraphInfo(const BoundExtraCreateRdfGraphInfo& other)
        : resourceInfo{other.resourceInfo.copy()}, literalInfo{other.literalInfo.copy()},
          resourceTripleInfo{other.resourceTripleInfo.copy()},
          literalTripleInfo{other.literalTripleInfo.copy()} {}

    inline std::unique_ptr<BoundExtraCreateCatalogEntryInfo> copy() const override {
        return std::make_unique<BoundExtraCreateRdfGraphInfo>(*this);
    }

    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<BoundExtraCreateRdfGraphInfo> deserialize(
        common::Deserializer& deserializer);
};

} // namespace binder
} // namespace kuzu

#include <memory>
#include <mutex>
#include <vector>


namespace kuzu {
namespace common {
class FileSystem;
enum class LogicalTypeID : uint8_t;
} // namespace common

namespace catalog {
class CatalogEntry;
} // namespace catalog

namespace function {
struct Function;
} // namespace function

namespace extension {
struct ExtensionUtils;
struct ExtensionOptions;
} // namespace extension

namespace storage {
class StorageExtension;
} // namespace storage

namespace main {
struct ExtensionOption;
class DatabaseManager;
class ClientContext;

/**
 * @brief Stores runtime configuration for creating or opening a Database
 */
struct KUZU_API SystemConfig {
    /**
     * @brief Creates a SystemConfig object.
     * @param bufferPoolSize Max size of the buffer pool in bytes.
     *        The larger the buffer pool, the more data from the database files is kept in memory,
     *        reducing the amount of File I/O
     * @param maxNumThreads The maximum number of threads to use during query execution
     * @param enableCompression Whether or not to compress data on-disk for supported types
     * @param readOnly If true, the database is opened read-only. No write transaction is
     * allowed on the `Database` object. Multiple read-only `Database` objects can be created with
     * the same database path. If false, the database is opened read-write. Under this mode,
     * there must not be multiple `Database` objects created with the same database path.
     * @param maxDBSize The maximum size of the database in bytes. Note that this is introduced
     * temporarily for now to get around with the default 8TB mmap address space limit some
     * environment. This will be removed once we implemente a better solution later. The value is
     * default to 1 << 43 (8TB) under 64-bit environment and 1GB under 32-bit one (see
     * `DEFAULT_VM_REGION_MAX_SIZE`).
     * @param autoCheckpoint If true, the database will automatically checkpoint when the size of
     * the WAL file exceeds the checkpoint threshold.
     * @param checkpointThreshold The threshold of the WAL file size in bytes. When the size of the
     * WAL file exceeds this threshold, the database will checkpoint if autoCheckpoint is true.
     */
    explicit SystemConfig(uint64_t bufferPoolSize = -1u, uint64_t maxNumThreads = 0,
        bool enableCompression = true, bool readOnly = false, uint64_t maxDBSize = -1u,
        bool autoCheckpoint = true, uint64_t checkpointThreshold = 16777216 /* 16MB */);

    uint64_t bufferPoolSize;
    uint64_t maxNumThreads;
    bool enableCompression;
    bool readOnly;
    uint64_t maxDBSize;
    bool autoCheckpoint;
    uint64_t checkpointThreshold;
};

/**
 * @brief Database class is the main class of KùzuDB. It manages all database components.
 */
class Database {
    friend class EmbeddedShell;
    friend class ClientContext;
    friend class Connection;
    friend class StorageDriver;
    friend class testing::BaseGraphTest;
    friend class testing::PrivateGraphTest;
    friend class transaction::TransactionContext;
    friend struct extension::ExtensionUtils;

public:
    /**
     * @brief Creates a database object.
     * @param databasePath Database path. If left empty, or :memory: is specified, this will create
     *        an in-memory database.
     * @param systemConfig System configurations (buffer pool size and max num threads).
     */
    KUZU_API explicit Database(std::string_view databasePath,
        SystemConfig systemConfig = SystemConfig());
    /**
     * @brief Destructs the database object.
     */
    KUZU_API ~Database();

    // TODO(Ziyi): Instead of exposing a dedicated API for adding a new function, we should consider
    // add function through the extension module.
    void addTableFunction(std::string name,
        std::vector<std::unique_ptr<function::Function>> functionSet);

    KUZU_API void registerFileSystem(std::unique_ptr<common::FileSystem> fs);

    KUZU_API void registerStorageExtension(std::string name,
        std::unique_ptr<storage::StorageExtension> storageExtension);

    KUZU_API void addExtensionOption(std::string name, common::LogicalTypeID type,
        common::Value defaultValue);

    KUZU_API catalog::Catalog* getCatalog() { return catalog.get(); }

    ExtensionOption* getExtensionOption(std::string name) const;

    const DBConfig& getConfig() const { return dbConfig; }

    common::case_insensitive_map_t<std::unique_ptr<storage::StorageExtension>>&
    getStorageExtensions();

    uint64_t getNextQueryID();

private:
    struct QueryIDGenerator {
        uint64_t queryID = 0;
        std::mutex queryIDLock;
    };

    void openLockFile();
    void initAndLockDBDir();

private:
    std::string databasePath;
    DBConfig dbConfig;
    std::unique_ptr<common::VirtualFileSystem> vfs;
    std::unique_ptr<storage::BufferManager> bufferManager;
    std::unique_ptr<storage::MemoryManager> memoryManager;
    std::unique_ptr<processor::QueryProcessor> queryProcessor;
    std::unique_ptr<catalog::Catalog> catalog;
    std::unique_ptr<storage::StorageManager> storageManager;
    std::unique_ptr<transaction::TransactionManager> transactionManager;
    std::unique_ptr<common::FileInfo> lockFile;
    std::unique_ptr<extension::ExtensionOptions> extensionOptions;
    std::unique_ptr<DatabaseManager> databaseManager;
    common::case_insensitive_map_t<std::unique_ptr<storage::StorageExtension>> storageExtensions;
    QueryIDGenerator queryIDGenerator;
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace function {

struct CastFunctionBindData : public FunctionBindData {
    // We don't allow configuring delimiters, ... in CAST function.
    // For performance purpose, we generate a default option object during binding time.
    common::CSVOption option;
    // TODO(Mahn): the following field should be removed once we refactor fixed list.
    uint64_t numOfEntries;

    explicit CastFunctionBindData(common::LogicalType dataType)
        : FunctionBindData{std::move(dataType)} {}

    inline std::unique_ptr<FunctionBindData> copy() const override {
        auto result = std::make_unique<CastFunctionBindData>(resultType.copy());
        result->numOfEntries = numOfEntries;
        result->option = option.copy();
        return result;
    }
};

} // namespace function
} // namespace kuzu

#include <memory>
#include <vector>


namespace kuzu {
namespace common {

// A DataChunk represents tuples as a set of value vectors and a selector array.
// The data chunk represents a subset of a relation i.e., a set of tuples as
// lists of the same length. It is appended into DataChunks and passed as intermediate
// representations between operators.
// A data chunk further contains a DataChunkState, which keeps the data chunk's size, selector, and
// currIdx (used when flattening and implies the value vector only contains the elements at currIdx
// of each value vector).
class DataChunk {
public:
    DataChunk() : DataChunk{0} {}
    explicit DataChunk(uint32_t numValueVectors)
        : DataChunk(numValueVectors, std::make_shared<DataChunkState>()){};

    DataChunk(uint32_t numValueVectors, const std::shared_ptr<DataChunkState>& state)
        : valueVectors(numValueVectors), state{state} {};
    DELETE_COPY_DEFAULT_MOVE(DataChunk);

    void insert(uint32_t pos, std::shared_ptr<ValueVector> valueVector);

    void resetAuxiliaryBuffer();

    inline uint32_t getNumValueVectors() const { return valueVectors.size(); }

    inline std::shared_ptr<ValueVector> getValueVector(uint64_t valueVectorPos) {
        return valueVectors[valueVectorPos];
    }

public:
    std::vector<std::shared_ptr<ValueVector>> valueVectors;
    std::shared_ptr<DataChunkState> state;
};

} // namespace common
} // namespace kuzu


namespace kuzu {
namespace function {

/**
 * Binary operator assumes function with null returns null. This does NOT applies to binary boolean
 * operations (e.g. AND, OR, XOR).
 */

struct BinaryFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* /*leftValueVector*/, common::ValueVector* /*rightValueVector*/,
        common::ValueVector* /*resultValueVector*/, uint64_t /*resultPos*/, void* /*dataPtr*/) {
        OP::operation(left, right, result);
    }
};

struct BinaryListStructFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* leftValueVector, common::ValueVector* rightValueVector,
        common::ValueVector* resultValueVector, uint64_t /*resultPos*/, void* /*dataPtr*/) {
        OP::operation(left, right, result, *leftValueVector, *rightValueVector, *resultValueVector);
    }
};

struct BinaryMapCreationFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* leftValueVector, common::ValueVector* rightValueVector,
        common::ValueVector* resultValueVector, uint64_t /*resultPos*/, void* dataPtr) {
        OP::operation(left, right, result, *leftValueVector, *rightValueVector, *resultValueVector,
            dataPtr);
    }
};

struct BinaryListExtractFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* leftValueVector, common::ValueVector* rightValueVector,
        common::ValueVector* resultValueVector, uint64_t resultPos, void* /*dataPtr*/) {
        OP::operation(left, right, result, *leftValueVector, *rightValueVector, *resultValueVector,
            resultPos);
    }
};

struct BinaryStringFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* /*leftValueVector*/, common::ValueVector* /*rightValueVector*/,
        common::ValueVector* resultValueVector, uint64_t /*resultPos*/, void* /*dataPtr*/) {
        OP::operation(left, right, result, *resultValueVector);
    }
};

struct BinaryComparisonFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* leftValueVector, common::ValueVector* rightValueVector,
        common::ValueVector* /*resultValueVector*/, uint64_t /*resultPos*/, void* /*dataPtr*/) {
        OP::operation(left, right, result, leftValueVector, rightValueVector);
    }
};

struct BinaryUDFFunctionWrapper {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        common::ValueVector* /*leftValueVector*/, common::ValueVector* /*rightValueVector*/,
        common::ValueVector* /*resultValueVector*/, uint64_t /*resultPos*/, void* dataPtr) {
        OP::operation(left, right, result, dataPtr);
    }
};

struct BinaryFunctionExecutor {
    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static inline void executeOnValue(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& resultValueVector, uint64_t lPos, uint64_t rPos, uint64_t resPos,
        void* dataPtr) {
        OP_WRAPPER::template operation<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC>(
            ((LEFT_TYPE*)left.getData())[lPos], ((RIGHT_TYPE*)right.getData())[rPos],
            ((RESULT_TYPE*)resultValueVector.getData())[resPos], &left, &right, &resultValueVector,
            resPos, dataPtr);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeBothFlat(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        auto lPos = left.state->getSelVector()[0];
        auto rPos = right.state->getSelVector()[0];
        auto resPos = result.state->getSelVector()[0];
        result.setNull(resPos, left.isNull(lPos) || right.isNull(rPos));
        if (!result.isNull(resPos)) {
            executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left, right,
                result, lPos, rPos, resPos, dataPtr);
        }
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeFlatUnFlat(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        auto lPos = left.state->getSelVector()[0];
        auto& rightSelVector = right.state->getSelVector();
        if (left.isNull(lPos)) {
            result.setAllNull();
        } else if (right.hasNoNullsGuarantee()) {
            if (rightSelVector.isUnfiltered()) {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                        right, result, lPos, i, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    auto rPos = rightSelVector[i];
                    executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                        right, result, lPos, rPos, rPos, dataPtr);
                }
            }
        } else {
            if (rightSelVector.isUnfiltered()) {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    result.setNull(i, right.isNull(i)); // left is always not null
                    if (!result.isNull(i)) {
                        executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                            right, result, lPos, i, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    auto rPos = rightSelVector[i];
                    result.setNull(rPos, right.isNull(rPos)); // left is always not null
                    if (!result.isNull(rPos)) {
                        executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                            right, result, lPos, rPos, rPos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeUnFlatFlat(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        auto rPos = right.state->getSelVector()[0];
        auto& leftSelVector = left.state->getSelVector();
        if (right.isNull(rPos)) {
            result.setAllNull();
        } else if (left.hasNoNullsGuarantee()) {
            if (leftSelVector.isUnfiltered()) {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                        right, result, i, rPos, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    auto lPos = leftSelVector[i];
                    executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                        right, result, lPos, rPos, lPos, dataPtr);
                }
            }
        } else {
            if (leftSelVector.isUnfiltered()) {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    result.setNull(i, left.isNull(i)); // right is always not null
                    if (!result.isNull(i)) {
                        executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                            right, result, i, rPos, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    auto lPos = leftSelVector[i];
                    result.setNull(lPos, left.isNull(lPos)); // right is always not null
                    if (!result.isNull(lPos)) {
                        executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                            right, result, lPos, rPos, lPos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeBothUnFlat(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(left.state == right.state);
        auto& resultSelVector = result.state->getSelVector();
        if (left.hasNoNullsGuarantee() && right.hasNoNullsGuarantee()) {
            if (resultSelVector.isUnfiltered()) {
                for (uint64_t i = 0; i < resultSelVector.getSelSize(); i++) {
                    executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                        right, result, i, i, i, dataPtr);
                }
            } else {
                for (uint64_t i = 0; i < resultSelVector.getSelSize(); i++) {
                    auto pos = resultSelVector[i];
                    executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                        right, result, pos, pos, pos, dataPtr);
                }
            }
        } else {
            if (resultSelVector.isUnfiltered()) {
                for (uint64_t i = 0; i < resultSelVector.getSelSize(); i++) {
                    result.setNull(i, left.isNull(i) || right.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                            right, result, i, i, i, dataPtr);
                    }
                }
            } else {
                for (uint64_t i = 0; i < resultSelVector.getSelSize(); i++) {
                    auto pos = resultSelVector[i];
                    result.setNull(pos, left.isNull(pos) || right.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left,
                            right, result, pos, pos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeSwitch(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        result.resetAuxiliaryBuffer();
        if (left.state->isFlat() && right.state->isFlat()) {
            executeBothFlat<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left, right,
                result, dataPtr);
        } else if (left.state->isFlat() && !right.state->isFlat()) {
            executeFlatUnFlat<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left, right,
                result, dataPtr);
        } else if (!left.state->isFlat() && right.state->isFlat()) {
            executeUnFlatFlat<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left, right,
                result, dataPtr);
        } else if (!left.state->isFlat() && !right.state->isFlat()) {
            executeBothUnFlat<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(left, right,
                result, dataPtr);
        } else {
            KU_ASSERT(false);
        }
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void execute(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryFunctionWrapper>(left, right,
            result, nullptr /* dataPtr */);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeString(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryStringFunctionWrapper>(left,
            right, result, nullptr /* dataPtr */);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeListStruct(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryListStructFunctionWrapper>(
            left, right, result, nullptr /* dataPtr */);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeMapCreation(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryMapCreationFunctionWrapper>(
            left, right, result, dataPtr);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeListExtract(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryListExtractFunctionWrapper>(
            left, right, result, nullptr /* dataPtr */);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeComparison(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryComparisonFunctionWrapper>(
            left, right, result, nullptr /* dataPtr */);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeUDF(common::ValueVector& left, common::ValueVector& right,
        common::ValueVector& result, void* dataPtr) {
        executeSwitch<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC, BinaryUDFFunctionWrapper>(left,
            right, result, dataPtr);
    }

    struct BinarySelectWrapper {
        template<typename LEFT_TYPE, typename RIGHT_TYPE, typename OP>
        static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, uint8_t& result,
            common::ValueVector* /*leftValueVector*/, common::ValueVector* /*rightValueVector*/) {
            OP::operation(left, right, result);
        }
    };

    struct BinaryComparisonSelectWrapper {
        template<typename LEFT_TYPE, typename RIGHT_TYPE, typename OP>
        static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, uint8_t& result,
            common::ValueVector* leftValueVector, common::ValueVector* rightValueVector) {
            OP::operation(left, right, result, leftValueVector, rightValueVector);
        }
    };

    template<class LEFT_TYPE, class RIGHT_TYPE, class FUNC, typename SELECT_WRAPPER>
    static void selectOnValue(common::ValueVector& left, common::ValueVector& right, uint64_t lPos,
        uint64_t rPos, uint64_t resPos, uint64_t& numSelectedValues,
        std::span<common::sel_t> selectedPositionsBuffer) {
        uint8_t resultValue = 0;
        SELECT_WRAPPER::template operation<LEFT_TYPE, RIGHT_TYPE, FUNC>(
            ((LEFT_TYPE*)left.getData())[lPos], ((RIGHT_TYPE*)right.getData())[rPos], resultValue,
            &left, &right);
        selectedPositionsBuffer[numSelectedValues] = resPos;
        numSelectedValues += (resultValue == true);
    }

    template<class LEFT_TYPE, class RIGHT_TYPE, class FUNC, typename SELECT_WRAPPER>
    static uint64_t selectBothFlat(common::ValueVector& left, common::ValueVector& right) {
        auto lPos = left.state->getSelVector()[0];
        auto rPos = right.state->getSelVector()[0];
        uint8_t resultValue = 0;
        if (!left.isNull(lPos) && !right.isNull(rPos)) {
            SELECT_WRAPPER::template operation<LEFT_TYPE, RIGHT_TYPE, FUNC>(
                ((LEFT_TYPE*)left.getData())[lPos], ((RIGHT_TYPE*)right.getData())[rPos],
                resultValue, &left, &right);
        }
        return resultValue == true;
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename FUNC, typename SELECT_WRAPPER>
    static bool selectFlatUnFlat(common::ValueVector& left, common::ValueVector& right,
        common::SelectionVector& selVector) {
        auto lPos = left.state->getSelVector()[0];
        uint64_t numSelectedValues = 0;
        auto selectedPositionsBuffer = selVector.getMultableBuffer();
        auto& rightSelVector = right.state->getSelVector();
        if (left.isNull(lPos)) {
            return numSelectedValues;
        } else if (right.hasNoNullsGuarantee()) {
            if (rightSelVector.isUnfiltered()) {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, lPos, i,
                        i, numSelectedValues, selectedPositionsBuffer);
                }
            } else {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    auto rPos = rightSelVector[i];
                    selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, lPos,
                        rPos, rPos, numSelectedValues, selectedPositionsBuffer);
                }
            }
        } else {
            if (rightSelVector.isUnfiltered()) {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    if (!right.isNull(i)) {
                        selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right,
                            lPos, i, i, numSelectedValues, selectedPositionsBuffer);
                    }
                }
            } else {
                for (auto i = 0u; i < rightSelVector.getSelSize(); ++i) {
                    auto rPos = rightSelVector[i];
                    if (!right.isNull(rPos)) {
                        selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right,
                            lPos, rPos, rPos, numSelectedValues, selectedPositionsBuffer);
                    }
                }
            }
        }
        selVector.setSelSize(numSelectedValues);
        return numSelectedValues > 0;
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename FUNC, typename SELECT_WRAPPER>
    static bool selectUnFlatFlat(common::ValueVector& left, common::ValueVector& right,
        common::SelectionVector& selVector) {
        auto rPos = right.state->getSelVector()[0];
        uint64_t numSelectedValues = 0;
        auto selectedPositionsBuffer = selVector.getMultableBuffer();
        auto& leftSelVector = left.state->getSelVector();
        if (right.isNull(rPos)) {
            return numSelectedValues;
        } else if (left.hasNoNullsGuarantee()) {
            if (leftSelVector.isUnfiltered()) {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, i, rPos,
                        i, numSelectedValues, selectedPositionsBuffer);
                }
            } else {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    auto lPos = leftSelVector[i];
                    selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, lPos,
                        rPos, lPos, numSelectedValues, selectedPositionsBuffer);
                }
            }
        } else {
            if (leftSelVector.isUnfiltered()) {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    if (!left.isNull(i)) {
                        selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, i,
                            rPos, i, numSelectedValues, selectedPositionsBuffer);
                    }
                }
            } else {
                for (auto i = 0u; i < leftSelVector.getSelSize(); ++i) {
                    auto lPos = leftSelVector[i];
                    if (!left.isNull(lPos)) {
                        selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right,
                            lPos, rPos, lPos, numSelectedValues, selectedPositionsBuffer);
                    }
                }
            }
        }
        selVector.setSelSize(numSelectedValues);
        return numSelectedValues > 0;
    }

    // Right, left, and result vectors share the same selectedPositions.
    template<class LEFT_TYPE, class RIGHT_TYPE, class FUNC, typename SELECT_WRAPPER>
    static bool selectBothUnFlat(common::ValueVector& left, common::ValueVector& right,
        common::SelectionVector& selVector) {
        uint64_t numSelectedValues = 0;
        auto selectedPositionsBuffer = selVector.getMultableBuffer();
        auto& leftSelVector = left.state->getSelVector();
        if (left.hasNoNullsGuarantee() && right.hasNoNullsGuarantee()) {
            if (leftSelVector.isUnfiltered()) {
                for (auto i = 0u; i < leftSelVector.getSelSize(); i++) {
                    selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, i, i, i,
                        numSelectedValues, selectedPositionsBuffer);
                }
            } else {
                for (auto i = 0u; i < leftSelVector.getSelSize(); i++) {
                    auto pos = leftSelVector[i];
                    selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, pos,
                        pos, pos, numSelectedValues, selectedPositionsBuffer);
                }
            }
        } else {
            if (leftSelVector.isUnfiltered()) {
                for (uint64_t i = 0; i < leftSelVector.getSelSize(); i++) {
                    auto isNull = left.isNull(i) || right.isNull(i);
                    if (!isNull) {
                        selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, i,
                            i, i, numSelectedValues, selectedPositionsBuffer);
                    }
                }
            } else {
                for (uint64_t i = 0; i < leftSelVector.getSelSize(); i++) {
                    auto pos = leftSelVector[i];
                    auto isNull = left.isNull(pos) || right.isNull(pos);
                    if (!isNull) {
                        selectOnValue<LEFT_TYPE, RIGHT_TYPE, FUNC, SELECT_WRAPPER>(left, right, pos,
                            pos, pos, numSelectedValues, selectedPositionsBuffer);
                    }
                }
            }
        }
        selVector.setSelSize(numSelectedValues);
        return numSelectedValues > 0;
    }

    // BOOLEAN (AND, OR, XOR)
    template<class LEFT_TYPE, class RIGHT_TYPE, class FUNC>
    static bool select(common::ValueVector& left, common::ValueVector& right,
        common::SelectionVector& selVector) {
        if (left.state->isFlat() && right.state->isFlat()) {
            return selectBothFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinarySelectWrapper>(left, right);
        } else if (left.state->isFlat() && !right.state->isFlat()) {
            return selectFlatUnFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinarySelectWrapper>(left, right,
                selVector);
        } else if (!left.state->isFlat() && right.state->isFlat()) {
            return selectUnFlatFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinarySelectWrapper>(left, right,
                selVector);
        } else {
            return selectBothUnFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinarySelectWrapper>(left, right,
                selVector);
        }
    }

    // COMPARISON (GT, GTE, LT, LTE, EQ, NEQ)
    template<class LEFT_TYPE, class RIGHT_TYPE, class FUNC>
    static bool selectComparison(common::ValueVector& left, common::ValueVector& right,
        common::SelectionVector& selVector) {
        if (left.state->isFlat() && right.state->isFlat()) {
            return selectBothFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinaryComparisonSelectWrapper>(left,
                right);
        } else if (left.state->isFlat() && !right.state->isFlat()) {
            return selectFlatUnFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinaryComparisonSelectWrapper>(
                left, right, selVector);
        } else if (!left.state->isFlat() && right.state->isFlat()) {
            return selectUnFlatFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinaryComparisonSelectWrapper>(
                left, right, selVector);
        } else {
            return selectBothUnFlat<LEFT_TYPE, RIGHT_TYPE, FUNC, BinaryComparisonSelectWrapper>(
                left, right, selVector);
        }
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace function {

struct ConstFunctionExecutor {

    template<typename RESULT_TYPE, typename OP>
    static void execute(common::ValueVector& result) {
        KU_ASSERT(result.state->isFlat());
        auto resultValues = (RESULT_TYPE*)result.getData();
        auto idx = result.state->getSelVector()[0];
        KU_ASSERT(idx == 0);
        OP::operation(resultValues[idx]);
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace function {

struct PointerFunctionExecutor {
    template<typename RESULT_TYPE, typename OP>
    static void execute(common::ValueVector& result, void* dataPtr) {
        if (result.state->getSelVector().isUnfiltered()) {
            for (auto i = 0u; i < result.state->getSelVector().getSelSize(); i++) {
                OP::operation(result.getValue<RESULT_TYPE>(i), dataPtr);
            }
        } else {
            for (auto i = 0u; i < result.state->getSelVector().getSelSize(); i++) {
                auto pos = result.state->getSelVector()[i];
                OP::operation(result.getValue<RESULT_TYPE>(pos), dataPtr);
            }
        }
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace function {

struct TernaryFunctionWrapper {
    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(A_TYPE& a, B_TYPE& b, C_TYPE& c, RESULT_TYPE& result,
        void* /*aValueVector*/, void* /*resultValueVector*/, void* /*dataPtr*/) {
        OP::operation(a, b, c, result);
    }
};

struct TernaryStringFunctionWrapper {
    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(A_TYPE& a, B_TYPE& b, C_TYPE& c, RESULT_TYPE& result,
        void* /*aValueVector*/, void* resultValueVector, void* /*dataPtr*/) {
        OP::operation(a, b, c, result, *(common::ValueVector*)resultValueVector);
    }
};

struct TernaryListFunctionWrapper {
    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(A_TYPE& a, B_TYPE& b, C_TYPE& c, RESULT_TYPE& result,
        void* aValueVector, void* resultValueVector, void* /*dataPtr*/) {
        OP::operation(a, b, c, result, *(common::ValueVector*)aValueVector,
            *(common::ValueVector*)resultValueVector);
    }
};

struct TernaryUDFFunctionWrapper {
    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename OP>
    static inline void operation(A_TYPE& a, B_TYPE& b, C_TYPE& c, RESULT_TYPE& result,
        void* /*aValueVector*/, void* /*resultValueVector*/, void* dataPtr) {
        OP::operation(a, b, c, result, dataPtr);
    }
};

struct TernaryFunctionExecutor {
    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeOnValue(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, uint64_t aPos, uint64_t bPos,
        uint64_t cPos, uint64_t resPos, void* dataPtr) {
        auto resValues = (RESULT_TYPE*)result.getData();
        OP_WRAPPER::template operation<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC>(
            ((A_TYPE*)a.getData())[aPos], ((B_TYPE*)b.getData())[bPos],
            ((C_TYPE*)c.getData())[cPos], resValues[resPos], (void*)&a, (void*)&result, dataPtr);
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeAllFlat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        auto aPos = a.state->getSelVector()[0];
        auto bPos = b.state->getSelVector()[0];
        auto cPos = c.state->getSelVector()[0];
        auto resPos = result.state->getSelVector()[0];
        result.setNull(resPos, a.isNull(aPos) || b.isNull(bPos) || c.isNull(cPos));
        if (!result.isNull(resPos)) {
            executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c, result,
                aPos, bPos, cPos, resPos, dataPtr);
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeFlatFlatUnflat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        auto aPos = a.state->getSelVector()[0];
        auto bPos = b.state->getSelVector()[0];
        auto& cSelVector = c.state->getSelVector();
        if (a.isNull(aPos) || b.isNull(bPos)) {
            result.setAllNull();
        } else if (c.hasNoNullsGuarantee()) {
            if (cSelVector.isUnfiltered()) {
                for (auto i = 0u; i < cSelVector.getSelSize(); ++i) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, aPos, bPos, i, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < cSelVector.getSelSize(); ++i) {
                    auto pos = cSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, aPos, bPos, pos, pos, dataPtr);
                }
            }
        } else {
            if (cSelVector.isUnfiltered()) {
                for (auto i = 0u; i < cSelVector.getSelSize(); ++i) {
                    result.setNull(i, c.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, aPos, bPos, i, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < cSelVector.getSelSize(); ++i) {
                    auto pos = cSelVector[i];
                    result.setNull(pos, c.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, aPos, bPos, pos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeFlatUnflatUnflat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(b.state == c.state);
        auto aPos = a.state->getSelVector()[0];
        auto& bSelVector = b.state->getSelVector();
        if (a.isNull(aPos)) {
            result.setAllNull();
        } else if (b.hasNoNullsGuarantee() && c.hasNoNullsGuarantee()) {
            if (bSelVector.isUnfiltered()) {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, aPos, i, i, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    auto pos = bSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, aPos, pos, pos, pos, dataPtr);
                }
            }
        } else {
            if (bSelVector.isUnfiltered()) {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    result.setNull(i, b.isNull(i) || c.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, aPos, i, i, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    auto pos = bSelVector[i];
                    result.setNull(pos, b.isNull(pos) || c.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, aPos, pos, pos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeFlatUnflatFlat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        auto aPos = a.state->getSelVector()[0];
        auto cPos = c.state->getSelVector()[0];
        auto& bSelVector = b.state->getSelVector();
        if (a.isNull(aPos) || c.isNull(cPos)) {
            result.setAllNull();
        } else if (b.hasNoNullsGuarantee()) {
            if (bSelVector.isUnfiltered()) {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, aPos, i, cPos, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    auto pos = bSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, aPos, pos, cPos, pos, dataPtr);
                }
            }
        } else {
            if (bSelVector.isUnfiltered()) {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    result.setNull(i, b.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, aPos, i, cPos, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < bSelVector.getSelSize(); ++i) {
                    auto pos = bSelVector[i];
                    result.setNull(pos, b.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, aPos, pos, cPos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeAllUnFlat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(a.state == b.state && b.state == c.state);
        auto& aSelVector = a.state->getSelVector();
        if (a.hasNoNullsGuarantee() && b.hasNoNullsGuarantee() && c.hasNoNullsGuarantee()) {
            if (aSelVector.isUnfiltered()) {
                for (uint64_t i = 0; i < aSelVector.getSelSize(); i++) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, i, i, i, i, dataPtr);
                }
            } else {
                for (uint64_t i = 0; i < aSelVector.getSelSize(); i++) {
                    auto pos = aSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, pos, pos, pos, pos, dataPtr);
                }
            }
        } else {
            if (aSelVector.isUnfiltered()) {
                for (uint64_t i = 0; i < aSelVector.getSelSize(); i++) {
                    result.setNull(i, a.isNull(i) || b.isNull(i) || c.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, i, i, i, i, dataPtr);
                    }
                }
            } else {
                for (uint64_t i = 0; i < aSelVector.getSelSize(); i++) {
                    auto pos = aSelVector[i];
                    result.setNull(pos, a.isNull(pos) || b.isNull(pos) || c.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, pos, pos, pos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeUnflatFlatFlat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        auto bPos = b.state->getSelVector()[0];
        auto cPos = c.state->getSelVector()[0];
        auto& aSelVector = a.state->getSelVector();
        if (b.isNull(bPos) || c.isNull(cPos)) {
            result.setAllNull();
        } else if (a.hasNoNullsGuarantee()) {
            if (aSelVector.isUnfiltered()) {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, i, bPos, cPos, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    auto pos = aSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, pos, bPos, cPos, pos, dataPtr);
                }
            }
        } else {
            if (aSelVector.isUnfiltered()) {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    result.setNull(i, a.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, i, bPos, cPos, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    auto pos = aSelVector[i];
                    result.setNull(pos, a.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, pos, bPos, cPos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeUnflatFlatUnflat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(a.state == c.state);
        auto& aSelVector = a.state->getSelVector();
        auto bPos = b.state->getSelVector()[0];
        if (b.isNull(bPos)) {
            result.setAllNull();
        } else if (a.hasNoNullsGuarantee() && c.hasNoNullsGuarantee()) {
            if (aSelVector.isUnfiltered()) {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, i, bPos, i, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    auto pos = aSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, pos, bPos, pos, pos, dataPtr);
                }
            }
        } else {
            if (aSelVector.isUnfiltered()) {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    result.setNull(i, a.isNull(i) || c.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, i, bPos, i, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    auto pos = b.state->getSelVector()[i];
                    result.setNull(pos, a.isNull(pos) || c.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, pos, bPos, pos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeUnflatUnFlatFlat(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(a.state == b.state);
        auto& aSelVector = a.state->getSelVector();
        auto cPos = c.state->getSelVector()[0];
        if (c.isNull(cPos)) {
            result.setAllNull();
        } else if (a.hasNoNullsGuarantee() && b.hasNoNullsGuarantee()) {
            if (aSelVector.isUnfiltered()) {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, i, i, cPos, i, dataPtr);
                }
            } else {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    auto pos = aSelVector[i];
                    executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                        result, pos, pos, cPos, pos, dataPtr);
                }
            }
        } else {
            if (aSelVector.isUnfiltered()) {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    result.setNull(i, a.isNull(i) || b.isNull(i));
                    if (!result.isNull(i)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, i, i, cPos, i, dataPtr);
                    }
                }
            } else {
                for (auto i = 0u; i < aSelVector.getSelSize(); ++i) {
                    auto pos = aSelVector[i];
                    result.setNull(pos, a.isNull(pos) || b.isNull(pos));
                    if (!result.isNull(pos)) {
                        executeOnValue<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b,
                            c, result, pos, pos, cPos, pos, dataPtr);
                    }
                }
            }
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC,
        typename OP_WRAPPER>
    static void executeSwitch(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result, void* dataPtr) {
        result.resetAuxiliaryBuffer();
        if (a.state->isFlat() && b.state->isFlat() && c.state->isFlat()) {
            executeAllFlat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c, result,
                dataPtr);
        } else if (a.state->isFlat() && b.state->isFlat() && !c.state->isFlat()) {
            executeFlatFlatUnflat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                result, dataPtr);
        } else if (a.state->isFlat() && !b.state->isFlat() && !c.state->isFlat()) {
            executeFlatUnflatUnflat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                result, dataPtr);
        } else if (a.state->isFlat() && !b.state->isFlat() && c.state->isFlat()) {
            executeFlatUnflatFlat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                result, dataPtr);
        } else if (!a.state->isFlat() && !b.state->isFlat() && !c.state->isFlat()) {
            executeAllUnFlat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c, result,
                dataPtr);
        } else if (!a.state->isFlat() && !b.state->isFlat() && c.state->isFlat()) {
            executeUnflatUnFlatFlat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                result, dataPtr);
        } else if (!a.state->isFlat() && b.state->isFlat() && c.state->isFlat()) {
            executeUnflatFlatFlat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                result, dataPtr);
        } else if (!a.state->isFlat() && b.state->isFlat() && !c.state->isFlat()) {
            executeUnflatFlatUnflat<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(a, b, c,
                result, dataPtr);
        } else {
            KU_ASSERT(false);
        }
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void execute(common::ValueVector& a, common::ValueVector& b, common::ValueVector& c,
        common::ValueVector& result) {
        executeSwitch<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, TernaryFunctionWrapper>(a, b, c,
            result, nullptr /* dataPtr */);
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeString(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result) {
        executeSwitch<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, TernaryStringFunctionWrapper>(a, b,
            c, result, nullptr /* dataPtr */);
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeListStruct(common::ValueVector& a, common::ValueVector& b,
        common::ValueVector& c, common::ValueVector& result) {
        executeSwitch<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, TernaryListFunctionWrapper>(a, b,
            c, result, nullptr /* dataPtr */);
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeUDF(common::ValueVector& a, common::ValueVector& b, common::ValueVector& c,
        common::ValueVector& result, void* dataPtr) {
        executeSwitch<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC, TernaryUDFFunctionWrapper>(a, b, c,
            result, dataPtr);
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu::main {
struct DBConfig;
} // namespace kuzu::main

namespace kuzu {
namespace main {
class AttachedKuzuDatabase;
} // namespace main

namespace binder {
struct BoundAlterInfo;
struct BoundCreateTableInfo;
struct BoundCreateSequenceInfo;
} // namespace binder

namespace common {
class VirtualFileSystem;
} // namespace common

namespace function {
struct ScalarMacroFunction;
} // namespace function

namespace storage {
class WAL;
} // namespace storage

namespace transaction {
class Transaction;
} // namespace transaction

namespace catalog {
class TableCatalogEntry;
class NodeTableCatalogEntry;
class RelTableCatalogEntry;
class RelGroupCatalogEntry;
class RDFGraphCatalogEntry;
class FunctionCatalogEntry;
class SequenceCatalogEntry;

class KUZU_API Catalog {
    friend class main::AttachedKuzuDatabase;

public:
    // This is extended by DuckCatalog and PostgresCatalog.
    Catalog();
    Catalog(const std::string& directory, common::VirtualFileSystem* vfs);
    virtual ~Catalog() = default;

    // ----------------------------- Table Schemas ----------------------------
    bool containsTable(transaction::Transaction* transaction, const std::string& tableName) const;

    common::table_id_t getTableID(transaction::Transaction* transaction,
        const std::string& tableName) const;
    std::vector<common::table_id_t> getNodeTableIDs(transaction::Transaction* transaction) const;
    std::vector<common::table_id_t> getRelTableIDs(transaction::Transaction* transaction) const;

    // TODO: Should remove this.
    std::string getTableName(transaction::Transaction* transaction,
        common::table_id_t tableID) const;
    TableCatalogEntry* getTableCatalogEntry(transaction::Transaction* transaction,
        const std::string& tableName) const;
    TableCatalogEntry* getTableCatalogEntry(transaction::Transaction* transaction,
        common::table_id_t tableID) const;
    std::vector<NodeTableCatalogEntry*> getNodeTableEntries(
        transaction::Transaction* transaction) const;
    std::vector<RelTableCatalogEntry*> getRelTableEntries(
        transaction::Transaction* transaction) const;
    std::vector<RelGroupCatalogEntry*> getRelTableGroupEntries(
        transaction::Transaction* transaction) const;
    std::vector<RDFGraphCatalogEntry*> getRdfGraphEntries(
        transaction::Transaction* transaction) const;
    std::vector<TableCatalogEntry*> getTableEntries(transaction::Transaction* transaction) const;
    std::vector<TableCatalogEntry*> getTableEntries(transaction::Transaction* transaction,
        const common::table_id_vector_t& tableIDs) const;
    bool tableInRDFGraph(transaction::Transaction* transaction, common::table_id_t tableID) const;
    bool tableInRelGroup(transaction::Transaction* transaction, common::table_id_t tableID) const;
    common::table_id_set_t getFwdRelTableIDs(transaction::Transaction* transaction,
        common::table_id_t nodeTableID) const;
    common::table_id_set_t getBwdRelTableIDs(transaction::Transaction* transaction,
        common::table_id_t nodeTableID) const;

    common::table_id_t createTableSchema(transaction::Transaction* transaction,
        const binder::BoundCreateTableInfo& info);
    void dropTableEntry(transaction::Transaction* transaction, std::string name);
    void dropTableEntry(transaction::Transaction* transaction, common::table_id_t tableID);
    void alterTableEntry(transaction::Transaction* transaction, const binder::BoundAlterInfo& info);

    // ----------------------------- Sequences ----------------------------
    bool containsSequence(transaction::Transaction* transaction,
        const std::string& sequenceName) const;

    common::sequence_id_t getSequenceID(transaction::Transaction* transaction,
        const std::string& sequenceName) const;
    SequenceCatalogEntry* getSequenceCatalogEntry(transaction::Transaction* transaction,
        common::sequence_id_t sequenceID) const;
    std::vector<SequenceCatalogEntry*> getSequenceEntries(
        transaction::Transaction* transaction) const;

    common::sequence_id_t createSequence(transaction::Transaction* transaction,
        const binder::BoundCreateSequenceInfo& info);
    void dropSequence(transaction::Transaction* transaction, std::string name);
    void dropSequence(transaction::Transaction* transaction, common::sequence_id_t sequenceID);

    static std::string genSerialName(const std::string& tableName, const std::string& propertyName);

    // ----------------------------- Types ----------------------------
    void createType(transaction::Transaction* transaction, std::string name,
        common::LogicalType type);
    common::LogicalType getType(transaction::Transaction*, std::string name);
    bool containsType(transaction::Transaction* transaction, const std::string& typeName);

    // ----------------------------- Functions ----------------------------
    void addFunction(transaction::Transaction* transaction, CatalogEntryType entryType,
        std::string name, function::function_set functionSet);
    void dropFunction(transaction::Transaction* transaction, const std::string& name);
    void addBuiltInFunction(CatalogEntryType entryType, std::string name,
        function::function_set functionSet);
    CatalogSet* getFunctions(transaction::Transaction* transaction) const;
    CatalogEntry* getFunctionEntry(transaction::Transaction* transaction, const std::string& name);
    std::vector<FunctionCatalogEntry*> getFunctionEntries(
        transaction::Transaction* transaction) const;

    bool containsMacro(transaction::Transaction* transaction, const std::string& macroName) const;
    void addScalarMacroFunction(transaction::Transaction* transaction, std::string name,
        std::unique_ptr<function::ScalarMacroFunction> macro);
    function::ScalarMacroFunction* getScalarMacroFunction(transaction::Transaction* transaction,
        const std::string& name) const;
    std::vector<std::string> getMacroNames(transaction::Transaction* transaction) const;

    void checkpoint(const std::string& databasePath, common::VirtualFileSystem* fs) const;

    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<Catalog*, TARGET*>(this);
    }

private:
    // The clientContext needs to be used when reading from a remote filesystem which
    // requires some user-specific configs (e.g. s3 username, password).
    void readFromFile(const std::string& directory, common::VirtualFileSystem* fs,
        common::FileVersionType versionType, main::ClientContext* context = nullptr);
    void saveToFile(const std::string& directory, common::VirtualFileSystem* fs,
        common::FileVersionType versionType) const;

private:
    // ----------------------------- Functions ----------------------------
    void registerBuiltInFunctions();

    // ----------------------------- Table entries ----------------------------

    void iterateCatalogEntries(transaction::Transaction* transaction,
        std::function<void(CatalogEntry*)> func) const {
        for (auto& [_, entry] : tables->getEntries(transaction)) {
            func(entry);
        }
    }
    template<typename T>
    std::vector<T*> getTableCatalogEntries(transaction::Transaction* transaction,
        CatalogEntryType catalogType) const {
        std::vector<T*> result;
        iterateCatalogEntries(transaction, [&](CatalogEntry* entry) {
            if (entry->getType() == catalogType) {
                result.push_back(common::ku_dynamic_cast<CatalogEntry*, T*>(entry));
            }
        });
        return result;
    }

    std::vector<common::table_id_t> getTableIDs(transaction::Transaction* transaction,
        CatalogEntryType catalogType) const;

    void alterRdfChildTableEntries(transaction::Transaction* transaction, CatalogEntry* entry,
        const binder::BoundAlterInfo& info) const;
    std::unique_ptr<CatalogEntry> createNodeTableEntry(transaction::Transaction* transaction,
        common::table_id_t tableID, const binder::BoundCreateTableInfo& info) const;
    std::unique_ptr<CatalogEntry> createRelTableEntry(transaction::Transaction* transaction,
        common::table_id_t tableID, const binder::BoundCreateTableInfo& info) const;
    std::unique_ptr<CatalogEntry> createRelTableGroupEntry(transaction::Transaction* transaction,
        common::table_id_t tableID, const binder::BoundCreateTableInfo& info);
    std::unique_ptr<CatalogEntry> createRdfGraphEntry(transaction::Transaction* transaction,
        common::table_id_t tableID, const binder::BoundCreateTableInfo& info);

    // ----------------------------- Sequence entries ----------------------------
    void iterateSequenceCatalogEntries(transaction::Transaction* transaction,
        std::function<void(CatalogEntry*)> func) const {
        for (auto& [_, entry] : sequences->getEntries(transaction)) {
            func(entry);
        }
    }

protected:
    std::unique_ptr<CatalogSet> tables;

private:
    std::unique_ptr<CatalogSet> sequences;
    std::unique_ptr<CatalogSet> functions;
    std::unique_ptr<CatalogSet> types;
};

} // namespace catalog
} // namespace kuzu

#include <array>


namespace kuzu {
namespace common {
class ValueVector;
} // namespace common

namespace transaction {
class Transaction;
} // namespace transaction

namespace storage {

class ColumnChunkData;
struct VectorUpdateInfo {
    common::transaction_t version;
    std::array<common::sel_t, common::DEFAULT_VECTOR_CAPACITY> rowsInVector;
    common::sel_t numRowsUpdated;
    // Older versions.
    std::unique_ptr<VectorUpdateInfo> prev;
    // Newer versions.
    VectorUpdateInfo* next;

    std::unique_ptr<ColumnChunkData> data;

    explicit VectorUpdateInfo(const common::transaction_t transactionID,
        common::LogicalType dataType)
        : version{transactionID}, rowsInVector{}, numRowsUpdated{0}, prev{nullptr}, next{nullptr} {
        data = ColumnChunkFactory::createColumnChunkData(std::move(dataType), false,
            common::DEFAULT_VECTOR_CAPACITY, ResidencyState::IN_MEMORY);
    }

    std::unique_ptr<VectorUpdateInfo> movePrev() { return std::move(prev); }
    void setPrev(std::unique_ptr<VectorUpdateInfo> prev) { this->prev = std::move(prev); }
    VectorUpdateInfo* getPrev() const { return prev.get(); }
    void setNext(VectorUpdateInfo* next) { this->next = next; }
    VectorUpdateInfo* getNext() const { return next; }
};

class UpdateInfo {
public:
    UpdateInfo() {}

    VectorUpdateInfo* update(const transaction::Transaction* transaction, common::idx_t vectorIdx,
        common::sel_t rowIdxInVector, const common::ValueVector& values);

    void setVectorInfo(common::idx_t vectorIdx, std::unique_ptr<VectorUpdateInfo> vectorInfo) {
        vectorsInfo[vectorIdx] = std::move(vectorInfo);
    }
    void clearVectorInfo(common::idx_t vectorIdx) { vectorsInfo[vectorIdx] = nullptr; }

    common::idx_t getNumVectors() const { return vectorsInfo.size(); }
    VectorUpdateInfo* getVectorInfo(const transaction::Transaction* transaction,
        common::idx_t idx) const;

    common::row_idx_t getNumUpdatedRows(const transaction::Transaction* transaction) const;

    bool hasUpdates(const transaction::Transaction* transaction, common::row_idx_t startRow,
        common::length_t numRows) const;

private:
    VectorUpdateInfo& getOrCreateVectorInfo(const transaction::Transaction* transaction,
        common::idx_t vectorIdx, common::sel_t rowIdxInVector, const common::LogicalType& dataType);

private:
    std::vector<std::unique_ptr<VectorUpdateInfo>> vectorsInfo;
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace storage {
class Table;
}

namespace main {

class ClientContext;
class KUZU_API StorageDriver {
public:
    explicit StorageDriver(Database* database);

    ~StorageDriver();

    void scan(const std::string& nodeName, const std::string& propertyName,
        common::offset_t* offsets, size_t numOffsets, uint8_t* result, size_t numThreads);

    uint64_t getNumNodes(const std::string& nodeName);
    uint64_t getNumRels(const std::string& relName);

private:
    void scanColumn(storage::Table* table, common::column_id_t columnID, common::offset_t* offsets,
        size_t size, uint8_t* result);

private:
    Database* database;
    std::unique_ptr<ClientContext> clientContext;
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace function {

/**
 * Unary operator assumes operation with null returns null. This does NOT applies to IS_NULL and
 * IS_NOT_NULL operation.
 */

struct UnaryFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static inline void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* /*dataPtr*/) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos),
            resultVector_.getValue<RESULT_TYPE>(resultPos));
    }
};

struct UnarySequenceFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static inline void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t /* resultPos */, void* dataPtr) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos), resultVector_, dataPtr);
    }
};

struct UnaryStringFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* /*dataPtr*/) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos),
            resultVector_.getValue<RESULT_TYPE>(resultPos), resultVector_);
    }
};

struct UnaryCastStringFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* dataPtr) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto resultVector_ = (common::ValueVector*)resultVector;
        // TODO(Ziyi): the reinterpret_cast is not safe since we don't always pass
        // CastFunctionBindData
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos),
            resultVector_->getValue<RESULT_TYPE>(resultPos), resultVector_, inputPos,
            &reinterpret_cast<CastFunctionBindData*>(dataPtr)->option);
    }
};

struct UnaryNestedTypeFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static inline void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* /*dataPtr*/) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos),
            resultVector_.getValue<RESULT_TYPE>(resultPos), inputVector_, resultVector_);
    }
};

struct UnaryCastFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* /*dataPtr*/) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos),
            resultVector_.getValue<RESULT_TYPE>(resultPos), inputVector_, resultVector_);
    }
};

struct UnaryRdfVariantCastFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* /*dataPtr*/) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::template operation<OPERAND_TYPE, RESULT_TYPE>(
            inputVector_.getValue<OPERAND_TYPE>(inputPos), inputVector_, inputPos,
            resultVector_.getValue<RESULT_TYPE>(resultPos), resultVector_, resultPos);
    }
};

struct UnaryUDFFunctionWrapper {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static inline void operation(void* inputVector, uint64_t inputPos, void* resultVector,
        uint64_t resultPos, void* dataPtr) {
        auto& inputVector_ = *(common::ValueVector*)inputVector;
        auto& resultVector_ = *(common::ValueVector*)resultVector;
        FUNC::operation(inputVector_.getValue<OPERAND_TYPE>(inputPos),
            resultVector_.getValue<RESULT_TYPE>(resultPos), dataPtr);
    }
};

struct UnaryFunctionExecutor {
    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC, typename OP_WRAPPER>
    static void executeOnValue(common::ValueVector& inputVector, uint64_t inputPos,
        common::ValueVector& resultVector, uint64_t resultPos, void* dataPtr) {
        OP_WRAPPER::template operation<OPERAND_TYPE, RESULT_TYPE, FUNC>((void*)&inputVector,
            inputPos, (void*)&resultVector, resultPos, dataPtr);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC, typename OP_WRAPPER>
    static void executeSwitch(common::ValueVector& operand, common::ValueVector& result,
        void* dataPtr) {
        result.resetAuxiliaryBuffer();
        auto& operandSelVector = operand.state->getSelVector();
        if (operand.state->isFlat()) {
            auto inputPos = operandSelVector[0];
            auto resultPos = result.state->getSelVector()[0];
            result.setNull(resultPos, operand.isNull(inputPos));
            if (!result.isNull(resultPos)) {
                executeOnValue<OPERAND_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(operand, inputPos,
                    result, resultPos, dataPtr);
            }
        } else {
            if (operand.hasNoNullsGuarantee()) {
                if (operandSelVector.isUnfiltered()) {
                    for (auto i = 0u; i < operandSelVector.getSelSize(); i++) {
                        executeOnValue<OPERAND_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(operand, i,
                            result, i, dataPtr);
                    }
                } else {
                    for (auto i = 0u; i < operandSelVector.getSelSize(); i++) {
                        auto pos = operandSelVector[i];
                        executeOnValue<OPERAND_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(operand, pos,
                            result, pos, dataPtr);
                    }
                }
            } else {
                if (operandSelVector.isUnfiltered()) {
                    for (auto i = 0u; i < operandSelVector.getSelSize(); i++) {
                        result.setNull(i, operand.isNull(i));
                        if (!result.isNull(i)) {
                            executeOnValue<OPERAND_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(operand, i,
                                result, i, dataPtr);
                        }
                    }
                } else {
                    for (auto i = 0u; i < operandSelVector.getSelSize(); i++) {
                        auto pos = operandSelVector[i];
                        result.setNull(pos, operand.isNull(pos));
                        if (!result.isNull(pos)) {
                            executeOnValue<OPERAND_TYPE, RESULT_TYPE, FUNC, OP_WRAPPER>(operand,
                                pos, result, pos, dataPtr);
                        }
                    }
                }
            }
        }
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void execute(common::ValueVector& operand, common::ValueVector& result) {
        executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC, UnaryFunctionWrapper>(operand, result,
            nullptr /* dataPtr */);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeUDF(common::ValueVector& operand, common::ValueVector& result,
        void* dataPtr) {
        executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC, UnaryUDFFunctionWrapper>(operand, result,
            dataPtr);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void executeSequence(common::ValueVector& operand, common::ValueVector& result,
        void* dataPtr) {
        result.resetAuxiliaryBuffer();
        auto inputPos = operand.state->getSelVector()[0];
        auto resultPos = result.state->getSelVector()[0];
        executeOnValue<OPERAND_TYPE, RESULT_TYPE, FUNC, UnarySequenceFunctionWrapper>(operand,
            inputPos, result, resultPos, dataPtr);
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace main {
class ClientContext;
}

namespace function {

struct TableFuncBindData;
struct TableFuncBindInput;

struct TableFuncSharedState {
    virtual ~TableFuncSharedState() = default;

    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<TableFuncSharedState*, TARGET*>(this);
    }
};

struct TableFuncLocalState {
    virtual ~TableFuncLocalState() = default;

    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<TableFuncLocalState*, TARGET*>(this);
    }
};

struct TableFuncInput {
    TableFuncBindData* bindData;
    TableFuncLocalState* localState;
    TableFuncSharedState* sharedState;

    TableFuncInput() = default;
    TableFuncInput(TableFuncBindData* bindData, TableFuncLocalState* localState,
        TableFuncSharedState* sharedState)
        : bindData{bindData}, localState{localState}, sharedState{sharedState} {}
    DELETE_COPY_DEFAULT_MOVE(TableFuncInput);
};

// We are in the middle of merging different scan operators into table function. But they organize
// output vectors in different ways. E.g.
// - Call functions and scan file functions put all vectors into single data chunk
// - Factorized table scan instead
// We introduce this as a temporary solution to unify the interface. In the long term, we should aim
// to use ResultSet as TableFuncOutput.
struct TableFuncOutput {
    common::DataChunk dataChunk;
    std::vector<common::ValueVector*> vectors;

    TableFuncOutput() = default;
    DELETE_COPY_DEFAULT_MOVE(TableFuncOutput);
};

struct TableFunctionInitInput {
    TableFuncBindData* bindData;

    explicit TableFunctionInitInput(TableFuncBindData* bindData) : bindData{bindData} {}

    virtual ~TableFunctionInitInput() = default;
};

using table_func_bind_t = std::function<std::unique_ptr<TableFuncBindData>(main::ClientContext*,
    function::TableFuncBindInput*)>;
using table_func_t = std::function<common::offset_t(TableFuncInput&, TableFuncOutput&)>;
using table_func_init_shared_t =
    std::function<std::unique_ptr<TableFuncSharedState>(TableFunctionInitInput&)>;
using table_func_init_local_t = std::function<std::unique_ptr<TableFuncLocalState>(
    TableFunctionInitInput&, TableFuncSharedState*, storage::MemoryManager*)>;
using table_func_can_parallel_t = std::function<bool()>;
using table_func_progress_t = std::function<double(TableFuncSharedState* sharedState)>;

struct KUZU_API TableFunction : public Function {
    table_func_t tableFunc;
    table_func_bind_t bindFunc;
    table_func_init_shared_t initSharedStateFunc;
    table_func_init_local_t initLocalStateFunc;
    table_func_can_parallel_t canParallelFunc = [] { return true; };
    table_func_progress_t progressFunc = [](TableFuncSharedState*) { return 0.0; };

    TableFunction()
        : Function{}, tableFunc{nullptr}, bindFunc{nullptr}, initSharedStateFunc{nullptr},
          initLocalStateFunc{nullptr} {};
    TableFunction(std::string name, table_func_t tableFunc, table_func_bind_t bindFunc,
        table_func_init_shared_t initSharedFunc, table_func_init_local_t initLocalFunc,
        std::vector<common::LogicalTypeID> inputTypes)
        : Function{std::move(name), std::move(inputTypes)}, tableFunc{tableFunc},
          bindFunc{bindFunc}, initSharedStateFunc{initSharedFunc},
          initLocalStateFunc{initLocalFunc} {}
    TableFunction(std::string name, table_func_t tableFunc, table_func_bind_t bindFunc,
        table_func_init_shared_t initSharedFunc, table_func_init_local_t initLocalFunc,
        table_func_progress_t progressFunc, std::vector<common::LogicalTypeID> inputTypes)
        : Function{std::move(name), std::move(inputTypes)}, tableFunc{tableFunc},
          bindFunc{bindFunc}, initSharedStateFunc{initSharedFunc},
          initLocalStateFunc{initLocalFunc}, progressFunc{progressFunc} {}

    std::string signatureToString() const override {
        return common::LogicalTypeUtils::toString(parameterTypeIDs);
    }

    std::unique_ptr<Function> copy() const override {
        return std::make_unique<TableFunction>(*this);
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace evaluator {
class ExpressionEvaluator;
} // namespace evaluator
namespace storage {

struct CompressionMetadata;

using read_values_to_vector_func_t =
    std::function<void(uint8_t* frame, PageCursor& pageCursor, common::ValueVector* resultVector,
        uint32_t posInVector, uint32_t numValuesToRead, const CompressionMetadata& metadata)>;
using write_values_from_vector_func_t = std::function<void(uint8_t* frame, uint16_t posInFrame,
    common::ValueVector* vector, uint32_t posInVector, const CompressionMetadata& metadata)>;
using write_values_func_t = std::function<void(uint8_t* frame, uint16_t posInFrame,
    const uint8_t* data, common::offset_t dataOffset, common::offset_t numValues,
    const CompressionMetadata& metadata, const common::NullMask*)>;

using read_values_to_page_func_t =
    std::function<void(uint8_t* frame, PageCursor& pageCursor, uint8_t* result,
        uint32_t posInResult, uint64_t numValues, const CompressionMetadata& metadata)>;
// This is a special usage for the `batchLookup` interface.
using batch_lookup_func_t = read_values_to_page_func_t;

class NullColumn;
class StructColumn;
class RelTableData;
struct ColumnCheckpointState;
class ShadowFile;
class BufferManager;
class Column {
    friend class StringColumn;
    friend class StructColumn;
    friend class ListColumn;
    friend class RelTableData;

public:
    // TODO(Guodong): Remove transaction from interface of Column. There is no need to be aware of
    // transaction when reading/writing from/to disk pages.
    Column(std::string name, common::LogicalType dataType, BMFileHandle* dataFH,
        BufferManager* bufferManager, ShadowFile* shadowFile, bool enableCompression,
        bool requireNullColumn = true);
    virtual ~Column();

    static std::unique_ptr<ColumnChunkData> flushChunkData(const ColumnChunkData& chunkData,
        BMFileHandle& dataFH);
    static std::unique_ptr<ColumnChunkData> flushNonNestedChunkData(
        const ColumnChunkData& chunkData, BMFileHandle& dataFH);
    static ColumnChunkMetadata flushData(const ColumnChunkData& chunkData, BMFileHandle& dataFH);

    virtual void scan(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t startOffsetInChunk, common::row_idx_t numValuesToScan,
        common::ValueVector* nodeIDVector, common::ValueVector* resultVector);
    virtual void lookupValue(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t nodeOffset, common::ValueVector* resultVector, uint32_t posInVector);

    // Scan from [startOffsetInGroup, endOffsetInGroup).
    virtual void scan(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t startOffsetInGroup, common::offset_t endOffsetInGroup,
        common::ValueVector* resultVector, uint64_t offsetInVector);
    // Scan from [startOffsetInGroup, endOffsetInGroup).
    virtual void scan(transaction::Transaction* transaction, const ChunkState& state,
        ColumnChunkData* columnChunk, common::offset_t startOffset = 0,
        common::offset_t endOffset = common::INVALID_OFFSET);

    common::LogicalType& getDataType() { return dataType; }
    const common::LogicalType& getDataType() const { return dataType; }

    Column* getNullColumn() const;

    std::string getName() const { return name; }

    virtual void scan(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t startOffsetInGroup, common::offset_t endOffsetInGroup, uint8_t* result);

    // Batch write to a set of sequential pages.
    virtual void write(ColumnChunkData& persistentChunk, ChunkState& state,
        common::offset_t dstOffset, ColumnChunkData* data, common::offset_t srcOffset,
        common::length_t numValues);

    // Append values to the end of the node group, resizing it if necessary
    common::offset_t appendValues(ColumnChunkData& persistentChunk, ChunkState& state,
        const uint8_t* data, const common::NullMask* nullChunkData, common::offset_t numValues);

    virtual void checkpointColumnChunk(ColumnCheckpointState& checkpointState);

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<Column&, TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET& cast() const {
        return common::ku_dynamic_cast<Column&, TARGET&>(*this);
    }

protected:
    virtual void scanInternal(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t startOffsetInChunk, common::row_idx_t numValuesToScan,
        common::ValueVector* nodeIDVector, common::ValueVector* resultVector);
    void scanUnfiltered(transaction::Transaction* transaction, PageCursor& pageCursor,
        uint64_t numValuesToScan, common::ValueVector* resultVector,
        const ColumnChunkMetadata& chunkMeta, uint64_t startPosInVector = 0) const;
    void scanFiltered(transaction::Transaction* transaction, PageCursor& pageCursor,
        uint64_t numValuesToScan, const common::SelectionVector& selVector,
        common::ValueVector* resultVector, const ColumnChunkMetadata& chunkMeta) const;

    virtual void lookupInternal(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t nodeOffset, common::ValueVector* resultVector, uint32_t posInVector);

    void readFromPage(transaction::Transaction* transaction, common::page_idx_t pageIdx,
        const std::function<void(uint8_t*)>& func) const;

    virtual void writeValues(ColumnChunkData& persistentChunk, ChunkState& state,
        common::offset_t dstOffset, const uint8_t* data, const common::NullMask* nullChunkData,
        common::offset_t srcOffset = 0, common::offset_t numValues = 1);

    // Produces a page cursor for the offset relative to the given node group
    static PageCursor getPageCursorForOffsetInGroup(common::offset_t offsetInChunk,
        const ChunkState& state);
    void updatePageWithCursor(PageCursor cursor,
        const std::function<void(uint8_t*, common::offset_t)>& writeOp) const;

    void updateStatistics(ColumnChunkMetadata& metadata, common::offset_t maxIndex,
        const std::optional<StorageValue>& min, const std::optional<StorageValue>& max) const;

protected:
    bool isMaxOffsetOutOfPagesCapacity(const ColumnChunkMetadata& metadata,
        common::offset_t maxOffset) const;

    virtual bool canCheckpointInPlace(const ChunkState& state,
        const ColumnCheckpointState& checkpointState);

    virtual void checkpointColumnChunkInPlace(ChunkState& state,
        const ColumnCheckpointState& checkpointState);
    void checkpointNullData(const ColumnCheckpointState& checkpointState) const;

    virtual void checkpointColumnChunkOutOfPlace(ChunkState& state,
        const ColumnCheckpointState& checkpointState);

    // check if val is in range [start, end)
    static bool isInRange(uint64_t val, uint64_t start, uint64_t end) {
        return val >= start && val < end;
    }

protected:
    std::string name;
    DBFileID dbFileID;
    common::LogicalType dataType;
    BMFileHandle* dataFH;
    BufferManager* bufferManager;
    ShadowFile* shadowFile;
    std::unique_ptr<NullColumn> nullColumn;
    read_values_to_vector_func_t readToVectorFunc;
    write_values_from_vector_func_t writeFromVectorFunc;
    write_values_func_t writeFunc;
    read_values_to_page_func_t readToPageFunc;
    batch_lookup_func_t batchLookupFunc;
    bool enableCompression;
};

class InternalIDColumn final : public Column {
public:
    InternalIDColumn(std::string name, BMFileHandle* dataFH, BufferManager* bufferManager,
        ShadowFile* shadowFile, bool enableCompression);

    void scan(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t startOffsetInChunk, common::row_idx_t numValuesToScan,
        common::ValueVector* nodeIDVector, common::ValueVector* resultVector) override {
        Column::scan(transaction, state, startOffsetInChunk, numValuesToScan, nodeIDVector,
            resultVector);
        populateCommonTableID(resultVector);
    }

    void scan(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t startOffsetInGroup, common::offset_t endOffsetInGroup,
        common::ValueVector* resultVector, uint64_t offsetInVector) override {
        Column::scan(transaction, state, startOffsetInGroup, endOffsetInGroup, resultVector,
            offsetInVector);
        populateCommonTableID(resultVector);
    }

    void lookupInternal(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t nodeOffset, common::ValueVector* resultVector,
        uint32_t posInVector) override {
        Column::lookupInternal(transaction, state, nodeOffset, resultVector, posInVector);
        populateCommonTableID(resultVector);
    }

    common::table_id_t getCommonTableID() const { return commonTableID; }
    // TODO(Guodong): This function should be removed through rewritting INTERNAL_ID as STRUCT.
    void setCommonTableID(common::table_id_t tableID) { commonTableID = tableID; }

private:
    void populateCommonTableID(const common::ValueVector* resultVector) const;

private:
    common::table_id_t commonTableID;
};

struct ColumnFactory {
    static std::unique_ptr<Column> createColumn(std::string name, common::LogicalType dataType,
        BMFileHandle* dataFH, BufferManager* bufferManager, ShadowFile* shadowFile,
        bool enableCompression);
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace storage {

struct ChunkCheckpointState {
    std::unique_ptr<ColumnChunkData> chunkData;
    common::row_idx_t startRow;
    common::length_t numRows;

    ChunkCheckpointState(std::unique_ptr<ColumnChunkData> chunkData, common::row_idx_t startRow,
        common::length_t numRows)
        : chunkData{std::move(chunkData)}, startRow{startRow}, numRows{numRows} {}
};

struct ColumnCheckpointState {
    ColumnChunkData& persistentData;
    std::vector<ChunkCheckpointState> chunkCheckpointStates;
    common::row_idx_t maxRowIdxToWrite;

    ColumnCheckpointState(ColumnChunkData& persistentData,
        std::vector<ChunkCheckpointState> chunkCheckpointStates)
        : persistentData{persistentData}, chunkCheckpointStates{std::move(chunkCheckpointStates)},
          maxRowIdxToWrite{0} {
        for (const auto& chunkCheckpointState : this->chunkCheckpointStates) {
            const auto endRowIdx = chunkCheckpointState.startRow + chunkCheckpointState.numRows;
            if (endRowIdx > maxRowIdxToWrite) {
                maxRowIdxToWrite = endRowIdx;
            }
        }
    }
};

class ColumnChunk {
public:
    ColumnChunk(const common::LogicalType& dataType, uint64_t capacity, bool enableCompression,
        ResidencyState residencyState);
    ColumnChunk(const common::LogicalType& dataType, bool enableCompression,
        ColumnChunkMetadata metadata);
    ColumnChunk(bool enableCompression, std::unique_ptr<ColumnChunkData> data);

    void initializeScanState(ChunkState& state) const;
    void scan(const transaction::Transaction* transaction, const ChunkState& state,
        common::ValueVector& nodeID, common::ValueVector& output, common::offset_t offsetInChunk,
        common::length_t length) const;
    template<ResidencyState SCAN_RESIDENCY_STATE>
    void scanCommitted(transaction::Transaction* transaction, ChunkState& chunkState,
        ColumnChunk& output, common::row_idx_t startRow = 0,
        common::row_idx_t numRows = common::INVALID_ROW_IDX) const;
    void lookup(transaction::Transaction* transaction, const ChunkState& state,
        common::offset_t rowInChunk, common::ValueVector& output,
        common::sel_t posInOutputVector) const;
    void update(const transaction::Transaction* transaction, common::offset_t offsetInChunk,
        const common::ValueVector& values);

    uint64_t getEstimatedMemoryUsage() const {
        return getResidencyState() == ResidencyState::ON_DISK ? 0 : data->getEstimatedMemoryUsage();
    }
    void serialize(common::Serializer& serializer) const;
    static std::unique_ptr<ColumnChunk> deserialize(common::Deserializer& deSer);

    uint64_t getNumValues() const { return data->getNumValues(); }
    void setNumValues(const uint64_t numValues) const { data->setNumValues(numValues); }

    common::row_idx_t getNumUpdatedRows(const transaction::Transaction* transaction) const;

    std::pair<std::unique_ptr<ColumnChunk>, std::unique_ptr<ColumnChunk>> scanUpdates(
        const transaction::Transaction* transaction) const;

    void setData(std::unique_ptr<ColumnChunkData> data) { this->data = std::move(data); }
    // Functions to access the in memory data.
    ColumnChunkData& getData() const { return *data; }
    const ColumnChunkData& getConstData() const { return *data; }
    std::unique_ptr<ColumnChunkData> moveData() { return std::move(data); }

    common::LogicalType& getDataType() { return data->getDataType(); }
    const common::LogicalType& getDataType() const { return data->getDataType(); }
    bool isCompressionEnabled() const { return enableCompression; }

    ResidencyState getResidencyState() const { return data->getResidencyState(); }
    bool hasUpdates() const { return updateInfo != nullptr; }
    bool hasUpdates(const transaction::Transaction* transaction, common::row_idx_t startRow,
        common::length_t numRows) const;
    // These functions should only work on in-memory and temporary column chunks.
    void resetToEmpty() const { data->resetToEmpty(); }
    void resetToAllNull() const { data->resetToAllNull(); }
    void resize(uint64_t newSize) const { data->resize(newSize); }
    void resetUpdateInfo() {
        if (updateInfo) {
            updateInfo.reset();
        }
    }

private:
    void scanCommittedUpdates(const transaction::Transaction* transaction, ColumnChunkData& output,
        common::offset_t startOffsetInOutput, common::row_idx_t startRowScanned,
        common::row_idx_t numRows) const;

private:
    // TODO(Guodong): This field should be removed. Ideally it shouldn't be cached anywhere in
    // storage structures, instead should be fed into functions needed from ClientContext dbConfig.
    bool enableCompression;
    std::unique_ptr<ColumnChunkData> data;
    // Update versions.
    std::unique_ptr<UpdateInfo> updateInfo;
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace function {

// Evaluate function at compile time, e.g. struct_extraction.
using scalar_func_compile_exec_t =
    std::function<void(FunctionBindData*, const std::vector<std::shared_ptr<common::ValueVector>>&,
        std::shared_ptr<common::ValueVector>&)>;
// Execute function.
using scalar_func_exec_t = std::function<void(
    const std::vector<std::shared_ptr<common::ValueVector>>&, common::ValueVector&, void*)>;
// Execute boolean function and write result to selection vector. Fast path for filter.
using scalar_func_select_t = std::function<bool(
    const std::vector<std::shared_ptr<common::ValueVector>>&, common::SelectionVector&)>;

struct ScalarFunction final : public BaseScalarFunction {
    scalar_func_exec_t execFunc;
    scalar_func_select_t selectFunc;
    scalar_func_compile_exec_t compileFunc;

    ScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_func_exec_t execFunc)
        : ScalarFunction{std::move(name), std::move(parameterTypeIDs), returnTypeID,
              std::move(execFunc), nullptr, nullptr, nullptr} {}

    ScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_func_exec_t execFunc,
        scalar_func_select_t selectFunc)
        : ScalarFunction{std::move(name), std::move(parameterTypeIDs), returnTypeID,
              std::move(execFunc), std::move(selectFunc), nullptr, nullptr} {}

    ScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_func_exec_t execFunc,
        scalar_func_select_t selectFunc, scalar_bind_func bindFunc)
        : ScalarFunction{std::move(name), std::move(parameterTypeIDs), returnTypeID,
              std::move(execFunc), std::move(selectFunc), nullptr, std::move(bindFunc)} {}

    ScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_func_exec_t execFunc,
        scalar_func_select_t selectFunc, scalar_func_compile_exec_t compileFunc,
        scalar_bind_func bindFunc)
        : BaseScalarFunction{std::move(name), std::move(parameterTypeIDs), returnTypeID,
              std::move(bindFunc)},
          execFunc{std::move(execFunc)}, selectFunc(std::move(selectFunc)),
          compileFunc{std::move(compileFunc)} {}

    ScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_bind_func bindFunc)
        : ScalarFunction{std::move(name), std::move(parameterTypeIDs), returnTypeID,
              nullptr /* execFunc */, nullptr /* selectFunc */, bindFunc} {}

    ScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypeIDs,
        common::LogicalTypeID returnTypeID, scalar_func_exec_t execFunc, scalar_bind_func bindFunc)
        : ScalarFunction{std::move(name), std::move(parameterTypeIDs), returnTypeID, execFunc,
              nullptr /* selectFunc */, bindFunc} {}

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void TernaryExecFunction(const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 3);
        TernaryFunctionExecutor::execute<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC>(*params[0],
            *params[1], *params[2], result);
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void TernaryStringExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 3);
        TernaryFunctionExecutor::executeString<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC>(
            *params[0], *params[1], *params[2], result);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void BinaryExecFunction(const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 2);
        BinaryFunctionExecutor::execute<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC>(*params[0],
            *params[1], result);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void BinaryStringExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 2);
        BinaryFunctionExecutor::executeString<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC>(*params[0],
            *params[1], result);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename FUNC>
    static bool BinarySelectFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::SelectionVector& selVector) {
        KU_ASSERT(params.size() == 2);
        return BinaryFunctionExecutor::select<LEFT_TYPE, RIGHT_TYPE, FUNC>(*params[0], *params[1],
            selVector);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC,
        typename EXECUTOR = UnaryFunctionExecutor>
    static void UnaryExecFunction(const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(params.size() == 1);
        EXECUTOR::template executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC, UnaryFunctionWrapper>(
            *params[0], result, dataPtr);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void UnarySequenceExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(params.size() == 1);
        UnaryFunctionExecutor::executeSequence<OPERAND_TYPE, RESULT_TYPE, FUNC>(*params[0], result,
            dataPtr);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void UnaryStringExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 1);
        UnaryFunctionExecutor::executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC,
            UnaryStringFunctionWrapper>(*params[0], result, nullptr /* dataPtr */);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC,
        typename EXECUTOR = UnaryFunctionExecutor>
    static void UnaryCastStringExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(params.size() == 1);
        EXECUTOR::template executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC,
            UnaryCastStringFunctionWrapper>(*params[0], result, dataPtr);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC,
        typename EXECUTOR = UnaryFunctionExecutor>
    static void UnaryCastExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(params.size() == 1);
        EXECUTOR::template executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC, UnaryCastFunctionWrapper>(
            *params[0], result, dataPtr);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC,
        typename EXECUTOR = UnaryFunctionExecutor>
    static void UnaryRdfVariantCastExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 1);
        EXECUTOR::template executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC,
            UnaryRdfVariantCastFunctionWrapper>(*params[0], result, nullptr /* dataPtr */);
    }

    template<typename OPERAND_TYPE, typename RESULT_TYPE, typename FUNC>
    static void UnaryExecNestedTypeFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 1);
        UnaryFunctionExecutor::executeSwitch<OPERAND_TYPE, RESULT_TYPE, FUNC,
            UnaryNestedTypeFunctionWrapper>(*params[0], result, nullptr /* dataPtr */);
    }

    template<typename RESULT_TYPE, typename FUNC>
    static void NullaryExecFunction(const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.empty());
        (void)params;
        ConstFunctionExecutor::execute<RESULT_TYPE, FUNC>(result);
    }

    template<typename RESULT_TYPE, typename FUNC>
    static void NullaryAuxilaryExecFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(params.empty());
        (void)params;
        PointerFunctionExecutor::execute<RESULT_TYPE, FUNC>(result, dataPtr);
    }

    template<typename A_TYPE, typename B_TYPE, typename C_TYPE, typename RESULT_TYPE, typename FUNC>
    static void TernaryExecListStructFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 3);
        TernaryFunctionExecutor::executeListStruct<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE, FUNC>(
            *params[0], *params[1], *params[2], result);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void BinaryExecListStructFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* /*dataPtr*/ = nullptr) {
        KU_ASSERT(params.size() == 2);
        BinaryFunctionExecutor::executeListStruct<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC>(
            *params[0], *params[1], result);
    }

    template<typename LEFT_TYPE, typename RIGHT_TYPE, typename RESULT_TYPE, typename FUNC>
    static void BinaryExecMapCreationFunction(
        const std::vector<std::shared_ptr<common::ValueVector>>& params,
        common::ValueVector& result, void* dataPtr) {
        KU_ASSERT(params.size() == 2);
        BinaryFunctionExecutor::executeMapCreation<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE, FUNC>(
            *params[0], *params[1], result, dataPtr);
    }

    std::unique_ptr<Function> copy() const override {
        return std::make_unique<ScalarFunction>(*this);
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace function {

struct ScanReplacementData {
    TableFunction func;
    TableFuncBindInput bindInput;
};

using scan_replace_func_t = std::function<std::unique_ptr<ScanReplacementData>(const std::string&)>;

struct ScanReplacement {
    explicit ScanReplacement(scan_replace_func_t replaceFunc) : replaceFunc{replaceFunc} {}

    scan_replace_func_t replaceFunc;
};

} // namespace function
} // namespace kuzu

#include <vector>


namespace kuzu {
namespace binder {
struct BoundExtraCreateCatalogEntryInfo;
} // namespace binder

namespace transaction {
class Transaction;
} // namespace transaction

namespace catalog {

class CatalogSet;
class KUZU_API TableCatalogEntry : public CatalogEntry {
public:
    //===--------------------------------------------------------------------===//
    // constructors
    //===--------------------------------------------------------------------===//
    TableCatalogEntry() = default;
    TableCatalogEntry(CatalogSet* set, CatalogEntryType catalogType, std::string name,
        common::table_id_t tableID)
        : CatalogEntry{catalogType, std::move(name)}, set{set}, tableID{tableID}, nextPID{0},
          nextColumnID{0} {}
    TableCatalogEntry& operator=(const TableCatalogEntry&) = delete;

    std::unique_ptr<TableCatalogEntry> alter(const binder::BoundAlterInfo& alterInfo);

    //===--------------------------------------------------------------------===//
    // getter & setter
    //===--------------------------------------------------------------------===//
    common::table_id_t getTableID() const { return tableID; }
    std::string getComment() const { return comment; }
    void setComment(std::string newComment) { comment = std::move(newComment); }
    virtual bool isParent(common::table_id_t /*tableID*/) { return false; };
    virtual common::TableType getTableType() const = 0;
    virtual function::TableFunction getScanFunction() { KU_UNREACHABLE; }
    binder::BoundAlterInfo* getAlterInfo() const { return alterInfo.get(); }
    void resetAlterInfo() { alterInfo = nullptr; }
    void setAlterInfo(const binder::BoundAlterInfo& alterInfo_) {
        alterInfo = std::make_unique<binder::BoundAlterInfo>(alterInfo_.copy());
    }

    //===--------------------------------------------------------------------===//
    // properties functions
    //===--------------------------------------------------------------------===//
    uint32_t getNumProperties() const { return properties.size(); }
    const std::vector<Property>& getPropertiesRef() const { return properties; }
    std::vector<Property>& getPropertiesUnsafe() { return properties; }
    bool containProperty(const std::string& propertyName) const;
    common::property_id_t getPropertyID(const std::string& propertyName) const;
    const Property* getProperty(common::property_id_t propertyID) const;
    uint32_t getPropertyPos(common::property_id_t propertyID) const;
    virtual common::column_id_t getColumnID(common::property_id_t propertyID) const;
    void addProperty(std::string propertyName, common::LogicalType dataType,
        std::unique_ptr<parser::ParsedExpression> defaultExpr);
    void dropProperty(common::property_id_t propertyID);
    void renameProperty(common::property_id_t propertyID, const std::string& newName);
    void resetColumnIDs();

    //===--------------------------------------------------------------------===//
    // serialization & deserialization
    //===--------------------------------------------------------------------===//
    void serialize(common::Serializer& serializer) const override;
    static std::unique_ptr<TableCatalogEntry> deserialize(common::Deserializer& deserializer,
        CatalogEntryType type);
    virtual std::unique_ptr<TableCatalogEntry> copy() const = 0;

    binder::BoundCreateTableInfo getBoundCreateTableInfo(
        transaction::Transaction* transaction) const;

protected:
    void copyFrom(const CatalogEntry& other) override;
    virtual std::unique_ptr<binder::BoundExtraCreateCatalogEntryInfo> getBoundExtraCreateInfo(
        transaction::Transaction* transaction) const = 0;

protected:
    CatalogSet* set;
    common::table_id_t tableID;
    std::string comment;
    common::property_id_t nextPID;
    common::column_id_t nextColumnID;
    std::vector<Property> properties;
    std::unique_ptr<binder::BoundAlterInfo> alterInfo;
};

struct TableCatalogEntryHasher {
    std::size_t operator()(TableCatalogEntry* entry) const {
        return std::hash<common::table_id_t>{}(entry->getTableID());
    }
};

struct TableCatalogEntryEquality {
    bool operator()(TableCatalogEntry* left, TableCatalogEntry* right) const {
        return left->getTableID() == right->getTableID();
    }
};

using table_catalog_entry_set_t =
    std::unordered_set<TableCatalogEntry*, TableCatalogEntryHasher, TableCatalogEntryEquality>;

} // namespace catalog
} // namespace kuzu

#include <atomic>


namespace kuzu {
namespace common {
class SelectionVector;
} // namespace common

namespace transaction {
class Transaction;
} // namespace transaction

namespace storage {

class Column;
struct TableScanState;
struct TableAddColumnState;
struct NodeGroupScanState;

enum class NodeGroupDataFormat : uint8_t { REGULAR = 0, CSR = 1 };

class ChunkedNodeGroup {
public:
    static constexpr uint64_t CHUNK_CAPACITY = 2048;

    ChunkedNodeGroup(std::vector<std::unique_ptr<ColumnChunk>> chunks,
        common::row_idx_t startRowIdx, NodeGroupDataFormat format = NodeGroupDataFormat::REGULAR);
    ChunkedNodeGroup(ChunkedNodeGroup& base,
        const std::vector<common::column_id_t>& selectedColumns);
    ChunkedNodeGroup(const std::vector<common::LogicalType>& columnTypes, bool enableCompression,
        uint64_t capacity, common::row_idx_t startRowIdx, ResidencyState residencyState,
        NodeGroupDataFormat format = NodeGroupDataFormat::REGULAR);
    virtual ~ChunkedNodeGroup() = default;

    common::idx_t getNumColumns() const { return chunks.size(); }
    common::row_idx_t getStartRowIdx() const { return startRowIdx; }
    common::row_idx_t getNumRows() const { return numRows; }
    common::row_idx_t getCapacity() const { return capacity; }
    const ColumnChunk& getColumnChunk(const common::column_id_t columnID) const {
        KU_ASSERT(columnID < chunks.size());
        return *chunks[columnID];
    }
    ColumnChunk& getColumnChunk(const common::column_id_t columnID) {
        KU_ASSERT(columnID < chunks.size());
        return *chunks[columnID];
    }
    std::unique_ptr<ColumnChunk> moveColumnChunk(const common::column_id_t columnID) {
        KU_ASSERT(columnID < chunks.size());
        return std::move(chunks[columnID]);
    }
    bool isFullOrOnDisk() const {
        return numRows == capacity || residencyState == ResidencyState::ON_DISK;
    }
    ResidencyState getResidencyState() const { return residencyState; }
    NodeGroupDataFormat getFormat() const { return format; }

    void resetToEmpty();
    void resetToAllNull() const;
    void resetNumRowsFromChunks();
    void setNumRows(common::offset_t numRows_);
    void resizeChunks(uint64_t newSize);
    void setVersionInfo(std::unique_ptr<VersionInfo> versionInfo) {
        this->versionInfo = std::move(versionInfo);
    }
    void resetVersionAndUpdateInfo();

    uint64_t append(const transaction::Transaction* transaction,
        const std::vector<common::ValueVector*>& columnVectors, common::row_idx_t startRowInVectors,
        uint64_t numValuesToAppend);
    // Appends up to numValuesToAppend from the other chunked node group, returning the actual
    // number of values appended.
    common::offset_t append(const transaction::Transaction* transaction,
        const ChunkedNodeGroup& other, common::offset_t offsetInOtherNodeGroup,
        common::offset_t numRowsToAppend);
    common::offset_t append(const transaction::Transaction* transaction,
        const std::vector<ColumnChunk*>& other, common::offset_t offsetInOtherNodeGroup,
        common::offset_t numRowsToAppend);
    void write(const ChunkedNodeGroup& data, common::column_id_t offsetColumnID);

    void scan(const transaction::Transaction* transaction, const TableScanState& scanState,
        const NodeGroupScanState& nodeGroupScanState, common::offset_t rowIdxInGroup,
        common::length_t numRowsToScan) const;

    template<ResidencyState SCAN_RESIDENCY_STATE>
    void scanCommitted(transaction::Transaction* transaction, TableScanState& scanState,
        NodeGroupScanState& nodeGroupScanState, ChunkedNodeGroup& output) const;

    bool hasUpdates() const;
    common::row_idx_t getNumDeletedRows(const transaction::Transaction* transaction) const;
    common::row_idx_t getNumUpdatedRows(const transaction::Transaction* transaction,
        common::column_id_t columnID);

    std::pair<std::unique_ptr<ColumnChunk>, std::unique_ptr<ColumnChunk>> scanUpdates(
        transaction::Transaction* transaction, common::column_id_t columnID);

    bool lookup(transaction::Transaction* transaction, const TableScanState& state,
        NodeGroupScanState& nodeGroupScanState, common::offset_t rowIdxInChunk,
        common::sel_t posInOutput) const;

    void update(transaction::Transaction* transaction, common::row_idx_t rowIdxInChunk,
        common::column_id_t columnID, const common::ValueVector& propertyVector);

    bool delete_(const transaction::Transaction* transaction, common::row_idx_t rowIdxInChunk);

    void addColumn(transaction::Transaction* transaction, const TableAddColumnState& addColumnState,
        bool enableCompression, BMFileHandle* dataFH);

    bool isDeleted(const transaction::Transaction* transaction, common::row_idx_t rowInChunk) const;
    bool isInserted(const transaction::Transaction* transaction,
        common::row_idx_t rowInChunk) const;
    bool hasAnyUpdates(const transaction::Transaction* transaction, common::column_id_t columnID,
        common::row_idx_t startRow, common::length_t numRows) const;
    common::row_idx_t getNumDeletions(const transaction::Transaction* transaction,
        common::row_idx_t startRow, common::length_t numRows) const;
    bool hasVersionInfo() const { return versionInfo != nullptr; }

    void finalize() const;

    virtual void writeToColumnChunk(common::idx_t chunkIdx, common::idx_t vectorIdx,
        const std::vector<std::unique_ptr<ColumnChunk>>& data, ColumnChunk& offsetChunk) {
        KU_ASSERT(residencyState != ResidencyState::ON_DISK);
        chunks[chunkIdx]->getData().write(&data[vectorIdx]->getData(), &offsetChunk.getData(),
            common::RelMultiplicity::ONE);
    }

    virtual std::unique_ptr<ChunkedNodeGroup> flushAsNewChunkedNodeGroup(
        transaction::Transaction* transaction, BMFileHandle& dataFH) const;
    virtual void flush(BMFileHandle& dataFH);

    uint64_t getEstimatedMemoryUsage() const;

    virtual void serialize(common::Serializer& serializer) const;
    static std::unique_ptr<ChunkedNodeGroup> deserialize(common::Deserializer& deSer);

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<ChunkedNodeGroup&, TARGET&>(*this);
    }
    template<class TARGETT>
    const TARGETT& cast() const {
        return common::ku_dynamic_cast<const ChunkedNodeGroup&, const TARGETT&>(*this);
    }

protected:
    NodeGroupDataFormat format;
    ResidencyState residencyState;
    common::row_idx_t startRowIdx;
    uint64_t capacity;
    std::atomic<common::row_idx_t> numRows;
    std::vector<std::unique_ptr<ColumnChunk>> chunks;
    std::unique_ptr<VersionInfo> versionInfo;
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace function {

struct UnaryUDFExecutor {
    template<class OPERAND_TYPE, class RESULT_TYPE>
    static inline void operation(OPERAND_TYPE& input, RESULT_TYPE& result, void* udfFunc) {
        typedef RESULT_TYPE (*unary_udf_func)(OPERAND_TYPE);
        auto unaryUDFFunc = (unary_udf_func)udfFunc;
        result = unaryUDFFunc(input);
    }
};

struct BinaryUDFExecutor {
    template<class LEFT_TYPE, class RIGHT_TYPE, class RESULT_TYPE>
    static inline void operation(LEFT_TYPE& left, RIGHT_TYPE& right, RESULT_TYPE& result,
        void* udfFunc) {
        typedef RESULT_TYPE (*binary_udf_func)(LEFT_TYPE, RIGHT_TYPE);
        auto binaryUDFFunc = (binary_udf_func)udfFunc;
        result = binaryUDFFunc(left, right);
    }
};

struct TernaryUDFExecutor {
    template<class A_TYPE, class B_TYPE, class C_TYPE, class RESULT_TYPE>
    static inline void operation(A_TYPE& a, B_TYPE& b, C_TYPE& c, RESULT_TYPE& result,
        void* udfFunc) {
        typedef RESULT_TYPE (*ternary_udf_func)(A_TYPE, B_TYPE, C_TYPE);
        auto ternaryUDFFunc = (ternary_udf_func)udfFunc;
        result = ternaryUDFFunc(a, b, c);
    }
};

struct UDF {
    template<typename T>
    static bool templateValidateType(const common::LogicalTypeID& type) {
        auto logicalType = common::LogicalType{type};
        auto physicalType = logicalType.getPhysicalType();
        auto physicalTypeMatch = common::TypeUtils::visit(physicalType,
            []<typename T1>(T1) { return std::is_same<T, T1>::value; });
        auto logicalTypeMatch = common::TypeUtils::visit(logicalType,
            []<typename T1>(T1) { return std::is_same<T, T1>::value; });
        return logicalTypeMatch || physicalTypeMatch;
    }

    template<typename T>
    static void validateType(const common::LogicalTypeID& type) {
        if (!templateValidateType<T>(type)) {
            throw common::CatalogException{
                "Incompatible udf parameter/return type and templated type."};
        }
    }

    template<typename RESULT_TYPE, typename... Args>
    static function::scalar_func_exec_t createEmptyParameterExecFunc(RESULT_TYPE (*)(Args...),
        const std::vector<common::LogicalTypeID>&) {
        KU_UNREACHABLE;
    }

    template<typename RESULT_TYPE>
    static function::scalar_func_exec_t createEmptyParameterExecFunc(RESULT_TYPE (*udfFunc)(),
        const std::vector<common::LogicalTypeID>&) {
        (void*)(udfFunc); // Disable compiler warnings.
        return [udfFunc](const std::vector<std::shared_ptr<common::ValueVector>>& params,
                   common::ValueVector& result, void* /*dataPtr*/ = nullptr) -> void {
            (void)params;
            KU_ASSERT(params.size() == 0);
            auto& resultSelVector = result.state->getSelVector();
            for (auto i = 0u; i < resultSelVector.getSelSize(); ++i) {
                auto resultPos = resultSelVector[i];
                result.copyFromValue(resultPos, common::Value(udfFunc()));
            }
        };
    }

    template<typename RESULT_TYPE, typename... Args>
    static function::scalar_func_exec_t createUnaryExecFunc(RESULT_TYPE (* /*udfFunc*/)(Args...),
        const std::vector<common::LogicalTypeID>& /*parameterTypes*/) {
        KU_UNREACHABLE;
    }

    template<typename RESULT_TYPE, typename OPERAND_TYPE>
    static function::scalar_func_exec_t createUnaryExecFunc(RESULT_TYPE (*udfFunc)(OPERAND_TYPE),
        const std::vector<common::LogicalTypeID>& parameterTypes) {
        if (parameterTypes.size() != 1) {
            throw common::CatalogException{
                "Expected exactly one parameter type for unary udf. Got: " +
                std::to_string(parameterTypes.size()) + "."};
        }
        validateType<OPERAND_TYPE>(parameterTypes[0]);
        function::scalar_func_exec_t execFunc =
            [udfFunc](const std::vector<std::shared_ptr<common::ValueVector>>& params,
                common::ValueVector& result, void* /*dataPtr*/ = nullptr) -> void {
            KU_ASSERT(params.size() == 1);
            UnaryFunctionExecutor::executeUDF<OPERAND_TYPE, RESULT_TYPE, UnaryUDFExecutor>(
                *params[0], result, (void*)udfFunc);
        };
        return execFunc;
    }

    template<typename RESULT_TYPE, typename... Args>
    static function::scalar_func_exec_t createBinaryExecFunc(RESULT_TYPE (* /*udfFunc*/)(Args...),
        const std::vector<common::LogicalTypeID>& /*parameterTypes*/) {
        KU_UNREACHABLE;
    }

    template<typename RESULT_TYPE, typename LEFT_TYPE, typename RIGHT_TYPE>
    static function::scalar_func_exec_t createBinaryExecFunc(
        RESULT_TYPE (*udfFunc)(LEFT_TYPE, RIGHT_TYPE),
        const std::vector<common::LogicalTypeID>& parameterTypes) {
        if (parameterTypes.size() != 2) {
            throw common::CatalogException{
                "Expected exactly two parameter types for binary udf. Got: " +
                std::to_string(parameterTypes.size()) + "."};
        }
        validateType<LEFT_TYPE>(parameterTypes[0]);
        validateType<RIGHT_TYPE>(parameterTypes[1]);
        function::scalar_func_exec_t execFunc =
            [udfFunc](const std::vector<std::shared_ptr<common::ValueVector>>& params,
                common::ValueVector& result, void* /*dataPtr*/ = nullptr) -> void {
            KU_ASSERT(params.size() == 2);
            BinaryFunctionExecutor::executeUDF<LEFT_TYPE, RIGHT_TYPE, RESULT_TYPE,
                BinaryUDFExecutor>(*params[0], *params[1], result, (void*)udfFunc);
        };
        return execFunc;
    }

    template<typename RESULT_TYPE, typename... Args>
    static function::scalar_func_exec_t createTernaryExecFunc(RESULT_TYPE (* /*udfFunc*/)(Args...),
        const std::vector<common::LogicalTypeID>& /*parameterTypes*/) {
        KU_UNREACHABLE;
    }

    template<typename RESULT_TYPE, typename A_TYPE, typename B_TYPE, typename C_TYPE>
    static function::scalar_func_exec_t createTernaryExecFunc(
        RESULT_TYPE (*udfFunc)(A_TYPE, B_TYPE, C_TYPE),
        std::vector<common::LogicalTypeID> parameterTypes) {
        if (parameterTypes.size() != 3) {
            throw common::CatalogException{
                "Expected exactly three parameter types for ternary udf. Got: " +
                std::to_string(parameterTypes.size()) + "."};
        }
        validateType<A_TYPE>(parameterTypes[0]);
        validateType<B_TYPE>(parameterTypes[1]);
        validateType<C_TYPE>(parameterTypes[2]);
        function::scalar_func_exec_t execFunc =
            [udfFunc](const std::vector<std::shared_ptr<common::ValueVector>>& params,
                common::ValueVector& result, void* /*dataPtr*/ = nullptr) -> void {
            KU_ASSERT(params.size() == 3);
            TernaryFunctionExecutor::executeUDF<A_TYPE, B_TYPE, C_TYPE, RESULT_TYPE,
                TernaryUDFExecutor>(*params[0], *params[1], *params[2], result, (void*)udfFunc);
        };
        return execFunc;
    }

    template<typename TR, typename... Args>
    static scalar_func_exec_t getScalarExecFunc(TR (*udfFunc)(Args...),
        std::vector<common::LogicalTypeID> parameterTypes) {
        constexpr auto numArgs = sizeof...(Args);
        switch (numArgs) {
        case 0:
            return createEmptyParameterExecFunc<TR, Args...>(udfFunc, std::move(parameterTypes));
        case 1:
            return createUnaryExecFunc<TR, Args...>(udfFunc, std::move(parameterTypes));
        case 2:
            return createBinaryExecFunc<TR, Args...>(udfFunc, std::move(parameterTypes));
        case 3:
            return createTernaryExecFunc<TR, Args...>(udfFunc, std::move(parameterTypes));
        default:
            throw common::BinderException("UDF function only supported until ternary!");
        }
    }

    template<typename T>
    static common::LogicalTypeID getParameterType() {
        if (std::is_same<T, bool>()) {
            return common::LogicalTypeID::BOOL;
        } else if (std::is_same<T, int8_t>()) {
            return common::LogicalTypeID::INT8;
        } else if (std::is_same<T, int16_t>()) {
            return common::LogicalTypeID::INT16;
        } else if (std::is_same<T, int32_t>()) {
            return common::LogicalTypeID::INT32;
        } else if (std::is_same<T, int64_t>()) {
            return common::LogicalTypeID::INT64;
        } else if (std::is_same<T, common::int128_t>()) {
            return common::LogicalTypeID::INT128;
        } else if (std::is_same<T, uint8_t>()) {
            return common::LogicalTypeID::UINT8;
        } else if (std::is_same<T, uint16_t>()) {
            return common::LogicalTypeID::UINT16;
        } else if (std::is_same<T, uint32_t>()) {
            return common::LogicalTypeID::UINT32;
        } else if (std::is_same<T, uint64_t>()) {
            return common::LogicalTypeID::UINT64;
        } else if (std::is_same<T, float>()) {
            return common::LogicalTypeID::FLOAT;
        } else if (std::is_same<T, double>()) {
            return common::LogicalTypeID::DOUBLE;
        } else if (std::is_same<T, common::ku_string_t>()) {
            return common::LogicalTypeID::STRING;
        } else {
            KU_UNREACHABLE;
        }
    }

    template<typename TA>
    static void getParameterTypesRecursive(std::vector<common::LogicalTypeID>& arguments) {
        arguments.push_back(getParameterType<TA>());
    }

    template<typename TA, typename TB, typename... Args>
    static void getParameterTypesRecursive(std::vector<common::LogicalTypeID>& arguments) {
        arguments.push_back(getParameterType<TA>());
        getParameterTypesRecursive<TB, Args...>(arguments);
    }

    template<typename... Args>
    static std::vector<common::LogicalTypeID> getParameterTypes() {
        std::vector<common::LogicalTypeID> parameterTypes;
        if constexpr (sizeof...(Args) > 0) {
            getParameterTypesRecursive<Args...>(parameterTypes);
        }
        return parameterTypes;
    }

    template<typename TR, typename... Args>
    static function_set getFunction(std::string name, TR (*udfFunc)(Args...),
        std::vector<common::LogicalTypeID> parameterTypes, common::LogicalTypeID returnType) {
        function_set definitions;
        if (returnType == common::LogicalTypeID::STRING) {
            KU_UNREACHABLE;
        }
        validateType<TR>(returnType);
        scalar_func_exec_t scalarExecFunc = getScalarExecFunc<TR, Args...>(udfFunc, parameterTypes);
        definitions.push_back(std::make_unique<function::ScalarFunction>(std::move(name),
            std::move(parameterTypes), returnType, std::move(scalarExecFunc)));
        return definitions;
    }

    template<typename TR, typename... Args>
    static function_set getFunction(std::string name, TR (*udfFunc)(Args...)) {
        return getFunction<TR, Args...>(std::move(name), udfFunc, getParameterTypes<Args...>(),
            getParameterType<TR>());
    }

    template<typename TR, typename... Args>
    static function_set getVectorizedFunction(std::string name, scalar_func_exec_t execFunc) {
        function_set definitions;
        definitions.push_back(std::make_unique<function::ScalarFunction>(std::move(name),
            getParameterTypes<Args...>(), getParameterType<TR>(), std::move(execFunc)));
        return definitions;
    }

    static function_set getVectorizedFunction(std::string name, scalar_func_exec_t execFunc,
        std::vector<common::LogicalTypeID> parameterTypes, common::LogicalTypeID returnType) {
        function_set definitions;
        definitions.push_back(std::make_unique<function::ScalarFunction>(std::move(name),
            std::move(parameterTypes), returnType, std::move(execFunc)));
        return definitions;
    }
};

} // namespace function
} // namespace kuzu


namespace kuzu {
namespace transaction {
class Transaction;
} // namespace transaction

namespace storage {

struct TableAddColumnState;
class NodeGroup;
struct NodeGroupScanState {
    // Index of committed but not yet checkpointed chunked group to scan.
    common::idx_t chunkedGroupIdx = 0;
    common::row_idx_t nextRowToScan = 0;
    // State of each chunk in the checkpointed chunked group.
    std::vector<ChunkState> chunkStates;

    explicit NodeGroupScanState(common::idx_t numChunks) { chunkStates.resize(numChunks); }
    virtual ~NodeGroupScanState() = default;
    DELETE_COPY_DEFAULT_MOVE(NodeGroupScanState);

    virtual void resetState() {
        chunkedGroupIdx = 0;
        nextRowToScan = 0;
        for (auto& chunkState : chunkStates) {
            chunkState.resetState();
        }
    }

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<NodeGroupScanState&, TARGET&>(*this);
    }
    template<class TARGETT>
    const TARGETT& constCast() {
        return common::ku_dynamic_cast<const NodeGroupScanState&, const TARGETT&>(*this);
    }
};

class MemoryManager;
struct NodeGroupCheckpointState {
    std::vector<common::column_id_t> columnIDs;
    std::vector<std::unique_ptr<Column>> columns;
    BMFileHandle& dataFH;
    MemoryManager* mm;

    NodeGroupCheckpointState(std::vector<common::column_id_t> columnIDs,
        std::vector<std::unique_ptr<Column>> columns, BMFileHandle& dataFH, MemoryManager* mm)
        : columnIDs{std::move(columnIDs)}, columns{std::move(columns)}, dataFH{dataFH}, mm{mm} {}
    virtual ~NodeGroupCheckpointState() = default;

    template<typename T>
    const T& cast() const {
        return common::ku_dynamic_cast<const NodeGroupCheckpointState&, const T&>(*this);
    }
    template<typename T>
    T& cast() {
        return common::ku_dynamic_cast<NodeGroupCheckpointState&, T&>(*this);
    }
};

struct NodeGroupScanResult {

    common::row_idx_t startRow = common::INVALID_ROW_IDX;
    common::row_idx_t numRows = 0;

    constexpr NodeGroupScanResult() noexcept = default;
    constexpr NodeGroupScanResult(common::row_idx_t startRow, common::row_idx_t numRows) noexcept
        : startRow{startRow}, numRows{numRows} {}

    bool operator==(const NodeGroupScanResult& other) const {
        return startRow == other.startRow && numRows == other.numRows;
    }
};

static auto NODE_GROUP_SCAN_EMMPTY_RESULT = NodeGroupScanResult{};

struct TableScanState;
class NodeGroup {
public:
    NodeGroup(const common::node_group_idx_t nodeGroupIdx, const bool enableCompression,
        std::vector<common::LogicalType> dataTypes,
        common::row_idx_t capacity = common::StorageConstants::NODE_GROUP_SIZE,
        NodeGroupDataFormat format = NodeGroupDataFormat::REGULAR)
        : nodeGroupIdx{nodeGroupIdx}, format{format}, enableCompression{enableCompression},
          numRows{0}, nextRowToAppend{0}, capacity{capacity}, dataTypes{std::move(dataTypes)} {}
    NodeGroup(const common::node_group_idx_t nodeGroupIdx, const bool enableCompression,
        std::unique_ptr<ChunkedNodeGroup> chunkedNodeGroup,
        common::row_idx_t capacity = common::StorageConstants::NODE_GROUP_SIZE,
        NodeGroupDataFormat format = NodeGroupDataFormat::REGULAR)
        : nodeGroupIdx{nodeGroupIdx}, format{format}, enableCompression{enableCompression},
          numRows{chunkedNodeGroup->getNumRows()}, nextRowToAppend{numRows}, capacity{capacity} {
        for (auto i = 0u; i < chunkedNodeGroup->getNumColumns(); i++) {
            dataTypes.push_back(chunkedNodeGroup->getColumnChunk(i).getDataType().copy());
        }
        const auto lock = chunkedGroups.lock();
        chunkedGroups.appendGroup(lock, std::move(chunkedNodeGroup));
    }
    NodeGroup(const common::node_group_idx_t nodeGroupIdx, const bool enableCompression,
        common::row_idx_t capacity, NodeGroupDataFormat format)
        : nodeGroupIdx{nodeGroupIdx}, format{format}, enableCompression{enableCompression},
          numRows{0}, nextRowToAppend{0}, capacity{capacity} {}
    virtual ~NodeGroup() = default;

    virtual bool isEmpty() const { return numRows.load() == 0; }
    virtual common::row_idx_t getNumRows() const { return numRows.load(); }
    void moveNextRowToAppend(common::row_idx_t numRowsToAppend) {
        nextRowToAppend += numRowsToAppend;
    }
    common::row_idx_t getNumRowsLeftToAppend() const { return capacity - nextRowToAppend; }
    bool isFull() const { return numRows.load() == capacity; }
    const std::vector<common::LogicalType>& getDataTypes() const { return dataTypes; }
    NodeGroupDataFormat getFormat() const { return format; }
    common::row_idx_t append(const transaction::Transaction* transaction,
        ChunkedNodeGroup& chunkedGroup, common::row_idx_t startRowIdx,
        common::row_idx_t numRowsToAppend);
    common::row_idx_t append(const transaction::Transaction* transaction,
        const std::vector<ColumnChunk*>& chunkedGroup, common::row_idx_t startRowIdx,
        common::row_idx_t numRowsToAppend);
    void append(const transaction::Transaction* transaction,
        const std::vector<common::ValueVector*>& vectors, common::row_idx_t startRowIdx,
        common::row_idx_t numRowsToAppend);

    void merge(transaction::Transaction* transaction,
        std::unique_ptr<ChunkedNodeGroup> chunkedGroup);

    virtual void initializeScanState(transaction::Transaction* transaction, TableScanState& state);
    void initializeScanState(transaction::Transaction* transaction, const common::UniqLock& lock,
        TableScanState& state);
    virtual NodeGroupScanResult scan(transaction::Transaction* transaction, TableScanState& state);

    bool lookup(const common::UniqLock& lock, transaction::Transaction* transaction,
        const TableScanState& state);
    bool lookup(transaction::Transaction* transaction, const TableScanState& state);

    void update(transaction::Transaction* transaction, common::row_idx_t rowIdxInGroup,
        common::column_id_t columnID, const common::ValueVector& propertyVector);
    bool delete_(const transaction::Transaction* transaction, common::row_idx_t rowIdxInGroup);

    common::row_idx_t getNumDeletedRows(const transaction::Transaction* transaction);
    virtual void addColumn(transaction::Transaction* transaction,
        TableAddColumnState& addColumnState, BMFileHandle* dataFH);

    void flush(BMFileHandle& dataFH);

    virtual void checkpoint(NodeGroupCheckpointState& state);

    bool hasChanges();
    uint64_t getEstimatedMemoryUsage();

    virtual void serialize(common::Serializer& serializer);
    static std::unique_ptr<NodeGroup> deserialize(common::Deserializer& deSer);

    common::node_group_idx_t getNumChunkedGroups() {
        const auto lock = chunkedGroups.lock();
        return chunkedGroups.getNumGroups(lock);
    }
    ChunkedNodeGroup* getChunkedNodeGroup(common::node_group_idx_t groupIdx) {
        const auto lock = chunkedGroups.lock();
        return chunkedGroups.getGroup(lock, groupIdx);
    }

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<NodeGroup&, TARGET&>(*this);
    }
    template<class TARGETT>
    const TARGETT& cast() const {
        return common::ku_dynamic_cast<const NodeGroup&, const TARGETT&>(*this);
    }

    bool isVisible(const transaction::Transaction* transaction, common::row_idx_t rowIdxInGroup);
    bool isDeleted(const transaction::Transaction* transaction, common::offset_t offsetInGroup);
    bool isInserted(const transaction::Transaction* transaction, common::offset_t offsetInGroup);

private:
    ChunkedNodeGroup* findChunkedGroupFromRowIdx(const common::UniqLock& lock,
        common::row_idx_t rowIdx);
    ChunkedNodeGroup* findChunkedGroupFromRowIdxNoLock(common::row_idx_t rowIdx);

    common::row_idx_t getNumDeletedRows(const common::UniqLock& lock);

    std::unique_ptr<ChunkedNodeGroup> checkpointInMemOnly(const common::UniqLock& lock,
        NodeGroupCheckpointState& state);
    std::unique_ptr<ChunkedNodeGroup> checkpointInMemAndOnDisk(const common::UniqLock& lock,
        NodeGroupCheckpointState& state);
    std::unique_ptr<VersionInfo> checkpointVersionInfo(const common::UniqLock& lock,
        const transaction::Transaction* transaction);

    template<ResidencyState SCAN_RESIDENCY_STATE>
    common::row_idx_t getNumResidentRows(const common::UniqLock& lock);
    template<ResidencyState SCAN_RESIDENCY_STATE>
    std::unique_ptr<ChunkedNodeGroup> scanAllInsertedAndVersions(const common::UniqLock& lock,
        const std::vector<common::column_id_t>& columnIDs, const std::vector<Column*>& columns);

    static void populateNodeID(common::ValueVector& nodeIDVector, common::table_id_t tableID,
        common::offset_t startNodeOffset, common::row_idx_t numRows);

protected:
    common::node_group_idx_t nodeGroupIdx;
    NodeGroupDataFormat format;
    bool enableCompression;
    std::atomic<common::row_idx_t> numRows;
    // `nextRowToAppend` is a cursor to allow us to pre-reserve a set of rows to append before
    // acutally appending data. This is an optimization to reduce lock-contention when appending in
    // parallel.
    // TODO(Guodong): Remove this field.
    common::row_idx_t nextRowToAppend;
    common::row_idx_t capacity;
    std::vector<common::LogicalType> dataTypes;
    GroupCollection<ChunkedNodeGroup> chunkedGroups;
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace evaluator {
class ExpressionEvaluator;
} // namespace evaluator
namespace storage {

enum class TableScanSource : uint8_t { COMMITTED = 0, UNCOMMITTED = 1, NONE = 3 };

struct TableScanState {
    std::unique_ptr<common::ValueVector> rowIdxVector;
    // Node/Rel ID vector. We assume all output vectors are within the same DataChunk as this one.
    common::ValueVector* IDVector;
    std::vector<common::ValueVector*> outputVectors;
    std::vector<common::column_id_t> columnIDs;
    common::NodeSemiMask* semiMask;

    // Only used when scan from persistent data.
    std::vector<Column*> columns;

    TableScanSource source = TableScanSource::NONE;
    common::node_group_idx_t nodeGroupIdx = common::INVALID_NODE_GROUP_IDX;
    NodeGroup* nodeGroup = nullptr;
    std::unique_ptr<NodeGroupScanState> nodeGroupScanState;

    std::vector<ColumnPredicateSet> columnPredicateSets;
    common::ZoneMapCheckResult zoneMapResult = common::ZoneMapCheckResult::ALWAYS_SCAN;

    explicit TableScanState(std::vector<common::column_id_t> columnIDs)
        : IDVector(nullptr), columnIDs{std::move(columnIDs)}, semiMask{nullptr} {
        rowIdxVector = std::make_unique<common::ValueVector>(common::LogicalType::INT64());
    }
    TableScanState(std::vector<common::column_id_t> columnIDs, std::vector<Column*> columns,
        std::vector<ColumnPredicateSet> columnPredicateSets)
        : IDVector(nullptr), columnIDs{std::move(columnIDs)}, semiMask{nullptr},
          columns{std::move(columns)}, columnPredicateSets{std::move(columnPredicateSets)} {
        rowIdxVector = std::make_unique<common::ValueVector>(common::LogicalType::INT64());
    }
    explicit TableScanState(std::vector<common::column_id_t> columnIDs,
        std::vector<Column*> columns)
        : IDVector(nullptr), columnIDs{std::move(columnIDs)}, semiMask{nullptr},
          columns{std::move(columns)} {
        rowIdxVector = std::make_unique<common::ValueVector>(common::LogicalType::INT64());
    }
    virtual ~TableScanState() = default;
    DELETE_COPY_DEFAULT_MOVE(TableScanState);

    virtual void resetState() {
        source = TableScanSource::NONE;
        nodeGroupIdx = common::INVALID_NODE_GROUP_IDX;
        nodeGroup = nullptr;
        nodeGroupScanState->resetState();
        zoneMapResult = common::ZoneMapCheckResult::ALWAYS_SCAN;
    }

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<TableScanState&, TARGET&>(*this);
    }
    template<class TARGETT>
    const TARGETT& cast() const {
        return common::ku_dynamic_cast<const TableScanState&, const TARGETT&>(*this);
    }
};

struct TableInsertState {
    const std::vector<common::ValueVector*>& propertyVectors;

    explicit TableInsertState(const std::vector<common::ValueVector*>& propertyVectors)
        : propertyVectors{propertyVectors} {}
    virtual ~TableInsertState() = default;

    template<typename T>
    const T& constCast() const {
        return common::ku_dynamic_cast<const TableInsertState&, const T&>(*this);
    }
    template<typename T>
    T& cast() {
        return common::ku_dynamic_cast<TableInsertState&, T&>(*this);
    }
};

struct TableUpdateState {
    common::column_id_t columnID;
    common::ValueVector& propertyVector;

    TableUpdateState(common::column_id_t columnID, common::ValueVector& propertyVector)
        : columnID{columnID}, propertyVector{propertyVector} {}
    virtual ~TableUpdateState() = default;

    template<typename T>
    const T& constCast() const {
        return common::ku_dynamic_cast<const TableUpdateState&, const T&>(*this);
    }
    template<typename T>
    T& cast() {
        return common::ku_dynamic_cast<TableUpdateState&, T&>(*this);
    }
};

struct TableDeleteState {
    virtual ~TableDeleteState() = default;

    template<typename T>
    const T& constCast() const {
        return common::ku_dynamic_cast<const TableDeleteState&, const T&>(*this);
    }
    template<typename T>
    T& cast() {
        return common::ku_dynamic_cast<TableDeleteState&, T&>(*this);
    }
};

struct TableAddColumnState final {
    const catalog::Property& property;
    evaluator::ExpressionEvaluator& defaultEvaluator;

    TableAddColumnState(const catalog::Property& property,
        evaluator::ExpressionEvaluator& defaultEvaluator)
        : property{property}, defaultEvaluator{defaultEvaluator} {}
    ~TableAddColumnState() = default;
};

class LocalTable;
class StorageManager;
class Table {
public:
    Table(const catalog::TableCatalogEntry* tableEntry, StorageManager* storageManager,
        MemoryManager* memoryManager);
    virtual ~Table() = default;

    static std::unique_ptr<Table> loadTable(common::Deserializer& deSer,
        const catalog::Catalog& catalog, StorageManager* storageManager,
        MemoryManager* memoryManager, common::VirtualFileSystem* vfs, main::ClientContext* context);

    common::TableType getTableType() const { return tableType; }
    common::table_id_t getTableID() const { return tableID; }
    std::string getTableName() const { return tableName; }
    BMFileHandle* getDataFH() const { return dataFH; }

    virtual void initializeScanState(transaction::Transaction* transaction,
        TableScanState& readState) = 0;
    bool scan(transaction::Transaction* transaction, TableScanState& scanState) {
        for (const auto& vector : scanState.outputVectors) {
            vector->resetAuxiliaryBuffer();
        }
        return scanInternal(transaction, scanState);
    }

    virtual void insert(transaction::Transaction* transaction, TableInsertState& insertState) = 0;
    virtual void update(transaction::Transaction* transaction, TableUpdateState& updateState) = 0;
    virtual bool delete_(transaction::Transaction* transaction, TableDeleteState& deleteState) = 0;

    virtual void addColumn(transaction::Transaction* transaction,
        TableAddColumnState& addColumnState) = 0;
    void dropColumn() { setHasChanges(); }

    virtual void commit(transaction::Transaction* transaction, LocalTable* localTable) = 0;
    virtual void checkpoint(common::Serializer& ser, catalog::TableCatalogEntry* tableEntry) = 0;

    virtual common::row_idx_t getNumRows() = 0;

    void setHasChanges() { hasChanges = true; }

    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<Table&, TARGET&>(*this);
    }
    template<class TARGET>
    const TARGET& cast() const {
        return common::ku_dynamic_cast<const Table&, const TARGET&>(*this);
    }
    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<Table*, TARGET*>(this);
    }

protected:
    virtual bool scanInternal(transaction::Transaction* transaction, TableScanState& scanState) = 0;

    virtual void serialize(common::Serializer& serializer) const;

    std::unique_ptr<common::DataChunk> constructDataChunk(
        const std::vector<common::LogicalType>& types);

protected:
    common::TableType tableType;
    common::table_id_t tableID;
    std::string tableName;
    bool enableCompression;
    BMFileHandle* dataFH;
    MemoryManager* memoryManager;
    BufferManager* bufferManager;
    ShadowFile* shadowFile;
    bool hasChanges;
};

} // namespace storage
} // namespace kuzu

#include <map>


namespace kuzu {
namespace transaction {
class Transaction;
} // namespace transaction

namespace storage {

using offset_to_row_idx_t = std::map<common::offset_t, common::row_idx_t>;
using offset_to_row_idx_vec_t = std::map<common::offset_t, std::vector<common::row_idx_t>>;
using offset_set_t = std::unordered_set<common::offset_t>;

struct TableAddColumnState;
struct TableInsertState;
struct TableUpdateState;
struct TableDeleteState;
class LocalTable {
public:
    virtual ~LocalTable() = default;

    virtual bool insert(transaction::Transaction* transaction, TableInsertState& insertState) = 0;
    virtual bool update(transaction::Transaction* transaction, TableUpdateState& updateState) = 0;
    virtual bool delete_(transaction::Transaction* transaction, TableDeleteState& deleteState) = 0;
    virtual bool addColumn(transaction::Transaction* transaction,
        TableAddColumnState& addColumnState) = 0;
    virtual void clear() = 0;
    virtual common::TableType getTableType() const = 0;
    virtual uint64_t getEstimatedMemUsage() = 0;

    template<class TARGET>
    const TARGET& constCast() {
        return common::ku_dynamic_cast<const LocalTable&, const TARGET&>(*this);
    }
    template<class TARGET>
    TARGET& cast() {
        return common::ku_dynamic_cast<LocalTable&, TARGET&>(*this);
    }
    template<class TARGET>
    TARGET* ptrCast() {
        return common::ku_dynamic_cast<LocalTable*, TARGET*>(this);
    }
    template<class TARGET>
    const TARGET* ptrCast() const {
        return common::ku_dynamic_cast<LocalTable*, TARGET*>(this);
    }

protected:
    explicit LocalTable(Table& table) : table{table} {}

protected:
    Table& table;
};

} // namespace storage
} // namespace kuzu

#include <unordered_map>


namespace kuzu {
namespace main {
class ClientContext;
} // namespace main
namespace storage {

class WAL;
// Data structures in LocalStorage are not thread-safe.
// For now, we only support single thread insertions and updates. Once we optimize them with
// multiple threads, LocalStorage and its related data structures should be reworked to be
// thread-safe.
class LocalStorage {
public:
    enum class NotExistAction { CREATE, RETURN_NULL };

    explicit LocalStorage(main::ClientContext& clientContext) : clientContext{clientContext} {}
    DELETE_COPY_AND_MOVE(LocalStorage);

    LocalTable* getLocalTable(common::table_id_t tableID,
        NotExistAction action = NotExistAction::RETURN_NULL);

    void commit();
    void rollback();

    uint64_t getEstimatedMemUsage() const;

private:
    main::ClientContext& clientContext;
    std::unordered_map<common::table_id_t, std::unique_ptr<LocalTable>> tables;
};

} // namespace storage
} // namespace kuzu


namespace kuzu {
namespace catalog {
class CatalogEntry;
class CatalogSet;
class SequenceCatalogEntry;
struct SequenceRollbackData;
} // namespace catalog
namespace main {
class ClientContext;
} // namespace main
namespace storage {
class LocalStorage;
class UndoBuffer;
class WAL;
} // namespace storage
namespace transaction {
class TransactionManager;

enum class TransactionType : uint8_t { READ_ONLY, WRITE, CHECKPOINT, DUMMY, RECOVERY };

class Transaction {
    friend class TransactionManager;

public:
    static constexpr common::transaction_t DUMMY_TRANSACTION_ID = 0;
    static constexpr common::transaction_t DUMMY_START_TIMESTAMP = 0;
    static constexpr common::transaction_t START_TRANSACTION_ID =
        static_cast<common::transaction_t>(1) << 63;

    Transaction(main::ClientContext& clientContext, TransactionType transactionType,
        common::transaction_t transactionID, common::transaction_t startTS);

    explicit Transaction(TransactionType transactionType) noexcept
        : type{transactionType}, ID{DUMMY_TRANSACTION_ID}, startTS{DUMMY_START_TIMESTAMP},
          commitTS{common::INVALID_TRANSACTION}, clientContext{nullptr}, undoBuffer{nullptr},
          forceCheckpoint{false} {
        currentTS = common::Timestamp::getCurrentTimestamp().value;
    }
    explicit Transaction(TransactionType transactionType, common::transaction_t ID,
        common::transaction_t startTS) noexcept
        : type{transactionType}, ID{ID}, startTS{startTS}, commitTS{common::INVALID_TRANSACTION},
          clientContext{nullptr}, undoBuffer{nullptr}, forceCheckpoint{false} {
        currentTS = common::Timestamp::getCurrentTimestamp().value;
    }

    TransactionType getType() const { return type; }
    bool isReadOnly() const { return TransactionType::READ_ONLY == type; }
    bool isWriteTransaction() const { return TransactionType::WRITE == type; }
    bool isDummy() const { return TransactionType::DUMMY == type; }
    bool isRecovery() const { return TransactionType::RECOVERY == type; }
    common::transaction_t getID() const { return ID; }
    common::transaction_t getStartTS() const { return startTS; }
    common::transaction_t getCommitTS() const { return commitTS; }
    int64_t getCurrentTS() const { return currentTS; }
    main::ClientContext* getClientContext() const { return clientContext; }

    void checkForceCheckpoint(common::StatementType statementType) {
        // Note: We always force checkpoint for COPY_FROM statement.
        if (statementType == common::StatementType::COPY_FROM) {
            forceCheckpoint = true;
        }
    }
    bool shouldAppendToUndoBuffer() const {
        return getID() > DUMMY_TRANSACTION_ID && !isReadOnly();
    }
    bool shouldLogToWAL() const;

    bool shouldForceCheckpoint() const;

    void commit(storage::WAL* wal) const;
    void rollback(storage::WAL* wal) const;

    uint64_t getEstimatedMemUsage() const;
    storage::LocalStorage* getLocalStorage() const { return localStorage.get(); }
    bool hasNewlyInsertedNodes(common::table_id_t tableID) const {
        return maxCommittedNodeOffsets.contains(tableID);
    }
    void setMaxCommittedNodeOffset(common::table_id_t tableID, common::offset_t offset) {
        maxCommittedNodeOffsets[tableID] = offset;
    }
    common::offset_t getMaxNodeOffsetBeforeCommit(common::table_id_t tableID) const {
        KU_ASSERT(maxCommittedNodeOffsets.contains(tableID));
        return maxCommittedNodeOffsets.at(tableID);
    }

    void pushCatalogEntry(catalog::CatalogSet& catalogSet, catalog::CatalogEntry& catalogEntry,
        bool skipLoggingToWAL = false) const;
    void pushSequenceChange(catalog::SequenceCatalogEntry* sequenceEntry, int64_t kCount,
        const catalog::SequenceRollbackData& data) const;
    void pushVectorInsertInfo(storage::VersionInfo& versionInfo, common::idx_t vectorIdx,
        common::row_idx_t startRowInVector, common::row_idx_t numRows) const;
    void pushVectorDeleteInfo(storage::VersionInfo& versionInfo, common::idx_t vectorIdx,
        common::row_idx_t startRowInVector, common::row_idx_t numRows) const;
    void pushVectorUpdateInfo(storage::UpdateInfo& updateInfo, common::idx_t vectorIdx,
        storage::VectorUpdateInfo& vectorUpdateInfo) const;

private:
    TransactionType type;
    common::transaction_t ID;
    common::transaction_t startTS;
    common::transaction_t commitTS;
    int64_t currentTS;
    main::ClientContext* clientContext;
    std::unique_ptr<storage::LocalStorage> localStorage;
    std::unique_ptr<storage::UndoBuffer> undoBuffer;
    bool forceCheckpoint;

    std::unordered_map<common::table_id_t, common::offset_t> maxCommittedNodeOffsets;
};

static auto DUMMY_TRANSACTION = Transaction(TransactionType::DUMMY);
static auto DUMMY_CHECKPOINT_TRANSACTION = Transaction(TransactionType::CHECKPOINT,
    Transaction::DUMMY_TRANSACTION_ID, Transaction::START_TRANSACTION_ID - 1);

} // namespace transaction
} // namespace kuzu

#include <mutex>


namespace kuzu {
namespace main {
class ClientContext;
}

namespace transaction {

/**
 * If the connection is in AUTO_COMMIT mode any query over the connection will be wrapped around
 * a transaction and committed (even if the query is READ_ONLY).
 * If the connection is in MANUAL transaction mode, which happens only if an application
 * manually begins a transaction (see below), then an application has to manually commit or
 * rollback the transaction by calling commit() or rollback().
 *
 * AUTO_COMMIT is the default mode when a Connection is created. If an application calls
 * begin[ReadOnly/Write]Transaction at any point, the mode switches to MANUAL. This creates
 * an "active transaction" in the connection. When a connection is in MANUAL mode and the
 * active transaction is rolled back or committed, then the active transaction is removed (so
 * the connection no longer has an active transaction) and the mode automatically switches
 * back to AUTO_COMMIT.
 * Note: When a Connection object is deconstructed, if the connection has an active (manual)
 * transaction, then the active transaction is rolled back.
 */
enum class TransactionMode : uint8_t { AUTO = 0, MANUAL = 1 };

class KUZU_API TransactionContext {
public:
    explicit TransactionContext(main::ClientContext& clientContext);
    ~TransactionContext();

    bool isAutoTransaction() const { return mode == TransactionMode::AUTO; }

    void beginReadTransaction();
    void beginWriteTransaction();
    void beginAutoTransaction(bool readOnlyStatement);
    void beginRecoveryTransaction();
    void validateManualTransaction(bool readOnlyStatement) const;

    void commit();
    void rollback();

    TransactionMode getTransactionMode() const { return mode; }
    bool hasActiveTransaction() const { return activeTransaction != nullptr; }
    Transaction* getActiveTransaction() const { return activeTransaction.get(); }

private:
    void clearTransaction();

private:
    void beginTransactionInternal(TransactionType transactionType);

private:
    std::mutex mtx;
    main::ClientContext& clientContext;
    TransactionMode mode;
    // TODO(Guodong): Should hold a raw pointer. Move ownership to TransactionManager.
    std::unique_ptr<Transaction> activeTransaction;
};

} // namespace transaction
} // namespace kuzu

#include <atomic>
#include <memory>
#include <mutex>


namespace kuzu {

namespace binder {
class Binder;
class ExpressionBinder;
} // namespace binder

namespace common {
class RandomEngine;
class TaskScheduler;
} // namespace common

namespace extension {
struct ExtensionOptions;
}

namespace main {
struct DBConfig;
class Database;
class DatabaseManager;
class AttachedKuzuDatabase;

struct ActiveQuery {
    explicit ActiveQuery();
    std::atomic<bool> interrupted;
    common::Timer timer;

    void reset();
};

/**
 * @brief Contain client side configuration. We make profiler associated per query, so profiler is
 * not maintained in client context.
 */
class KUZU_API ClientContext {
    friend class Connection;
    friend class binder::Binder;
    friend class binder::ExpressionBinder;

public:
    explicit ClientContext(Database* database);
    ~ClientContext();

    // Client config
    const ClientConfig* getClientConfig() const { return &clientConfig; }
    ClientConfig* getClientConfigUnsafe() { return &clientConfig; }
    const DBConfig* getDBConfig() const { return &dbConfig; }
    DBConfig* getDBConfigUnsafe() { return &dbConfig; }
    common::Value getCurrentSetting(const std::string& optionName);
    bool isOptionSet(const std::string& optionName) const;
    // Timer and timeout
    void interrupt() { activeQuery.interrupted = true; }
    bool interrupted() const { return activeQuery.interrupted; }
    bool hasTimeout() const { return clientConfig.timeoutInMS != 0; }
    void setQueryTimeOut(uint64_t timeoutInMS);
    uint64_t getQueryTimeOut() const;
    void startTimer();
    uint64_t getTimeoutRemainingInMS() const;
    void resetActiveQuery() { activeQuery.reset(); }

    // Parallelism
    void setMaxNumThreadForExec(uint64_t numThreads);
    uint64_t getMaxNumThreadForExec() const;

    // Transaction.
    transaction::Transaction* getTx() const;
    transaction::TransactionContext* getTransactionContext() const;

    // Progress bar
    common::ProgressBar* getProgressBar() const;

    // Replace function.
    void addScanReplace(function::ScanReplacement scanReplacement);
    std::unique_ptr<function::ScanReplacementData> tryReplace(const std::string& objectName) const;
    // Extension
    void setExtensionOption(std::string name, common::Value value);
    extension::ExtensionOptions* getExtensionOptions() const;
    std::string getExtensionDir() const;

    // Environment.
    std::string getEnvVariable(const std::string& name);

    // Database component getters.
    std::string getDatabasePath() const;
    Database* getDatabase() const { return localDatabase; }
    common::TaskScheduler* getTaskScheduler() const;
    DatabaseManager* getDatabaseManager() const;
    storage::StorageManager* getStorageManager() const;
    storage::MemoryManager* getMemoryManager();
    storage::WAL* getWAL() const;
    catalog::Catalog* getCatalog() const;
    transaction::TransactionManager* getTransactionManagerUnsafe() const;
    common::VirtualFileSystem* getVFSUnsafe() const;
    common::RandomEngine* getRandomEngine();

    // Query.
    std::unique_ptr<PreparedStatement> prepare(std::string_view query);
    std::unique_ptr<QueryResult> executeWithParams(PreparedStatement* preparedStatement,
        std::unordered_map<std::string, std::unique_ptr<common::Value>> inputParams,
        std::optional<uint64_t> queryID = std::nullopt);
    std::unique_ptr<QueryResult> query(std::string_view queryStatement,
        std::optional<uint64_t> queryID = std::nullopt);
    void runQuery(std::string query);

    // TODO(Jiamin): should remove after supporting ddl in manual tx
    std::unique_ptr<PreparedStatement> prepareTest(std::string_view query);
    // only use for test framework
    std::vector<std::shared_ptr<parser::Statement>> parseQuery(std::string_view query);

    void setDefaultDatabase(AttachedKuzuDatabase* defaultDatabase_);
    bool hasDefaultDatabase();

    void runFuncInTransaction(const std::function<void(void)>& fun);
    void addScalarFunction(std::string name, function::function_set definitions);
    void removeScalarFunction(std::string name);

    void cleanUP();

private:
    std::unique_ptr<QueryResult> query(std::string_view query, std::string_view encodedJoin,
        bool enumerateAllPlans = true, std::optional<uint64_t> queryID = std::nullopt);

    std::unique_ptr<QueryResult> queryResultWithError(std::string_view errMsg);

    std::unique_ptr<PreparedStatement> preparedStatementWithError(std::string_view errMsg);

    // when we do prepare, we will start a transaction for the query
    // when we execute after prepare in a same context, we set requireNewTx to false and will not
    // commit the transaction in prepare when we only prepare a query statement, we set requireNewTx
    // to true and will commit the transaction in prepare
    std::unique_ptr<PreparedStatement> prepareNoLock(
        std::shared_ptr<parser::Statement> parsedStatement, bool enumerateAllPlans = false,
        std::string_view joinOrder = std::string_view(), bool requireNewTx = true,
        std::optional<std::unordered_map<std::string, std::shared_ptr<common::Value>>> inputParams =
            std::nullopt);

    template<typename T, typename... Args>
    std::unique_ptr<QueryResult> executeWithParams(PreparedStatement* preparedStatement,
        std::unordered_map<std::string, std::unique_ptr<common::Value>> params,
        std::pair<std::string, T> arg, std::pair<std::string, Args>... args) {
        auto name = arg.first;
        auto val = std::make_unique<common::Value>((T)arg.second);
        params.insert({name, std::move(val)});
        return executeWithParams(preparedStatement, std::move(params), args...);
    }

    void bindParametersNoLock(PreparedStatement* preparedStatement,
        const std::unordered_map<std::string, std::unique_ptr<common::Value>>& inputParams);

    std::unique_ptr<QueryResult> executeAndAutoCommitIfNecessaryNoLock(
        PreparedStatement* preparedStatement, uint32_t planIdx = 0u,
        std::optional<uint64_t> queryID = std::nullopt);

    bool canExecuteWriteQuery();

    // Client side configurable settings.
    ClientConfig clientConfig;
    // Database configurable settings.
    DBConfig& dbConfig;
    // Current query.
    ActiveQuery activeQuery;
    // Transaction context.
    std::unique_ptr<transaction::TransactionContext> transactionContext;
    // Replace external object as pointer Value;
    std::vector<function::ScanReplacement> scanReplacements;
    // Extension configurable settings.
    std::unordered_map<std::string, common::Value> extensionOptionValues;
    // Random generator for UUID.
    std::unique_ptr<common::RandomEngine> randomEngine;
    // Local database.
    Database* localDatabase;
    // Remote database.
    AttachedKuzuDatabase* remoteDatabase;
    // Progress bar.
    std::unique_ptr<common::ProgressBar> progressBar;
    std::mutex mtx;
};

} // namespace main
} // namespace kuzu


namespace kuzu {
namespace main {

/**
 * @brief Connection is used to interact with a Database instance. Each Connection is thread-safe.
 * Multiple connections can connect to the same Database instance in a multi-threaded environment.
 */
class Connection {
    friend class kuzu::testing::BaseGraphTest;
    friend class kuzu::testing::PrivateGraphTest;
    friend class kuzu::testing::TestHelper;
    friend class kuzu::testing::TestRunner;
    friend class kuzu::benchmark::Benchmark;
    friend class kuzu::testing::TinySnbDDLTest;
    friend class ConnectionExecuteAsyncWorker;
    friend class ConnectionQueryAsyncWorker;

public:
    /**
     * @brief Creates a connection to the database.
     * @param database A pointer to the database instance that this connection will be connected to.
     */
    KUZU_API explicit Connection(Database* database);
    /**
     * @brief Destructs the connection.
     */
    KUZU_API ~Connection();
    /**
     * @brief Sets the maximum number of threads to use for execution in the current connection.
     * @param numThreads The number of threads to use for execution in the current connection.
     */
    KUZU_API void setMaxNumThreadForExec(uint64_t numThreads);
    /**
     * @brief Returns the maximum number of threads to use for execution in the current connection.
     * @return the maximum number of threads to use for execution in the current connection.
     */
    KUZU_API uint64_t getMaxNumThreadForExec();

    /**
     * @brief Executes the given query and returns the result.
     * @param query The query to execute.
     * @return the result of the query.
     */
    KUZU_API std::unique_ptr<QueryResult> query(std::string_view query);

    /**
     * @brief Prepares the given query and returns the prepared statement.
     * @param query The query to prepare.
     * @return the prepared statement.
     */
    KUZU_API std::unique_ptr<PreparedStatement> prepare(std::string_view query);
    /**
     * @brief Executes the given prepared statement with args and returns the result.
     * @param preparedStatement The prepared statement to execute.
     * @param args The parameter pack where each arg is a std::pair with the first element being
     * parameter name and second element being parameter value.
     * @return the result of the query.
     */
    template<typename... Args>
    inline std::unique_ptr<QueryResult> execute(PreparedStatement* preparedStatement,
        std::pair<std::string, Args>... args) {
        std::unordered_map<std::string, std::unique_ptr<common::Value>> inputParameters;
        return executeWithParams(preparedStatement, std::move(inputParameters), args...);
    }
    /**
     * @brief Executes the given prepared statement with inputParams and returns the result.
     * @param preparedStatement The prepared statement to execute.
     * @param inputParams The parameter pack where each arg is a std::pair with the first element
     * being parameter name and second element being parameter value.
     * @return the result of the query.
     */
    KUZU_API std::unique_ptr<QueryResult> executeWithParams(PreparedStatement* preparedStatement,
        std::unordered_map<std::string, std::unique_ptr<common::Value>> inputParams);
    /**
     * @brief interrupts all queries currently executing within this connection.
     */
    KUZU_API void interrupt();

    /**
     * @brief sets the query timeout value of the current connection. A value of zero (the default)
     * disables the timeout.
     */
    KUZU_API void setQueryTimeOut(uint64_t timeoutInMS);

    // Note: this function throws exception if creating scalar function fails.
    template<typename TR, typename... Args>
    void createScalarFunction(std::string name, TR (*udfFunc)(Args...)) {
        addScalarFunction(name, function::UDF::getFunction<TR, Args...>(name, udfFunc));
    }

    // Note: this function throws exception if creating scalar function fails.
    template<typename TR, typename... Args>
    void createScalarFunction(std::string name, std::vector<common::LogicalTypeID> parameterTypes,
        common::LogicalTypeID returnType, TR (*udfFunc)(Args...)) {
        addScalarFunction(name, function::UDF::getFunction<TR, Args...>(name, udfFunc,
                                    std::move(parameterTypes), returnType));
    }

    void addUDFFunctionSet(std::string name, function::function_set func) {
        addScalarFunction(name, std::move(func));
    }

    void removeUDFFunction(std::string name) { removeScalarFunction(name); }

    template<typename TR, typename... Args>
    void createVectorizedFunction(std::string name, function::scalar_func_exec_t scalarFunc) {
        addScalarFunction(name,
            function::UDF::getVectorizedFunction<TR, Args...>(name, std::move(scalarFunc)));
    }

    void createVectorizedFunction(std::string name,
        std::vector<common::LogicalTypeID> parameterTypes, common::LogicalTypeID returnType,
        function::scalar_func_exec_t scalarFunc) {
        addScalarFunction(name, function::UDF::getVectorizedFunction(name, std::move(scalarFunc),
                                    std::move(parameterTypes), returnType));
    }

    ClientContext* getClientContext() { return clientContext.get(); };

private:
    std::unique_ptr<QueryResult> query(std::string_view query, std::string_view encodedJoin,
        bool enumerateAllPlans = true);

    std::unique_ptr<QueryResult> queryResultWithError(std::string_view errMsg);

    std::unique_ptr<PreparedStatement> preparedStatementWithError(std::string_view errMsg);

    std::unique_ptr<PreparedStatement> prepareNoLock(
        std::shared_ptr<parser::Statement> parsedStatement, bool enumerateAllPlans = false,
        std::string_view joinOrder = std::string_view());

    template<typename T, typename... Args>
    std::unique_ptr<QueryResult> executeWithParams(PreparedStatement* preparedStatement,
        std::unordered_map<std::string, std::unique_ptr<common::Value>> params,
        std::pair<std::string, T> arg, std::pair<std::string, Args>... args) {
        return clientContext->executeWithParams(preparedStatement, std::move(params), arg, args...);
    }

    void bindParametersNoLock(PreparedStatement* preparedStatement,
        const std::unordered_map<std::string, std::unique_ptr<common::Value>>& inputParams);

    std::unique_ptr<QueryResult> executeAndAutoCommitIfNecessaryNoLock(
        PreparedStatement* preparedStatement, uint32_t planIdx = 0u);

    KUZU_API void addScalarFunction(std::string name, function::function_set definitions);
    KUZU_API void removeScalarFunction(std::string name);

    std::unique_ptr<QueryResult> queryWithID(std::string_view query, uint64_t queryID);

    std::unique_ptr<QueryResult> executeWithParamsWithID(PreparedStatement* preparedStatement,
        std::unordered_map<std::string, std::unique_ptr<common::Value>> inputParams,
        uint64_t queryID);

private:
    Database* database;
    std::unique_ptr<ClientContext> clientContext;
};

} // namespace main
} // namespace kuzu

