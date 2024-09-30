#include <vector>
#include <string>
#include <iostream>

#include <string.h> // this is where strerror lives, apparently.
#include <fcntl.h>
#include <functional>
#include <format>
#include <errno.h>
#include <sys/ioctl.h>
#include <sstream>
#include <linux/videodev2.h>

#include "reactor.hpp"

static char *isA = "is a";
static char *format = "format";
static char *hasFormat = "has format";
static char *hasFormatName = "has format name";
static char *hasId = "has id";
static char* camera = "camera";

////////////////////////////////////////////////////////////////////////////////

// I cannot stress
//   just how insane
//     I find this API.
//
// Because it's a lot.
//   Yes, it is that much.
//     jesus christ.
template<typename T, unsigned long fmt>
void v4lEnumerate(int fd, T &desc, std::function<void(const T &)> forEach) {
    int res = ioctl(fd, fmt, &desc);
    if (res >= 0) {
        forEach(desc);
        desc.index++;
        return v4lEnumerate<T, fmt>(fd, desc, forEach);
    }

    // If we get EINVAL, that's actually totally fine! Because who the hell designed this library?!
    // I mean that's fine & we just stop calling ioctl. Otherwise, there's an actual error.
    if (res < 0 && errno != EINVAL) {
        throw new std::runtime_error(
                 std::format("Error when trying to enumerate {0}: {1}", typeid(T).name(), strerror(errno)));
    }
}


////////////////////////////////////////////////////////////////////////////////

Tuple GetQuery() {
    return Tuple {
        { TupleNoun::Tag::Query },
        isA,
        { TupleNoun::Tag::Str, { .str = camera }}
    };
}

std::vector<Tuple>* WhenHandler(Tuple* result) {
    std::cout << "Hello from CameraHandler" << std::endl;
    std::cout << "    Subject is " << result->subject << std::endl;

    auto outTuples = new std::vector<Tuple>();

    auto camName = result->subject.dat.str;

    int fd = open(camName, O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error(
                std::format("ioctl failed with error: {0}", strerror(errno)));
    }

    struct v4l2_fmtdesc fmtdesc {
            .index = 0,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    };

    v4lEnumerate<struct v4l2_fmtdesc, VIDIOC_ENUM_FMT>(fd, fmtdesc, [&camName, &result, &outTuples](auto desc) {
        std::cout << "Enum'd format [" << desc.index << "]: " << desc.description << std::endl;
        std::stringstream ss;
        ss << desc.description;

        outTuples->push_back(Tuple {
            result->subject,
            hasFormat,
            TupleNoun::fromUint(desc.pixelformat),
        });

        outTuples->push_back(Tuple {
            TupleNoun::fromUint(desc.pixelformat),
            hasFormatName,
            TupleNoun::fromString(ss.str())
        });
    });

    return outTuples;
}

extern "C" void free_tuple_obj(void* obj) {}

extern "C" void free_tuple_subj(void* subj) {}

