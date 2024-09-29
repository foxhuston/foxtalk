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

static char *hasFormat = "has format";
static char *hasWidth = "has width";
static char *hasHeight = "has height";

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
        hasFormat,
        { TupleNoun::Tag::Query }
    };
}

std::vector<Tuple>* WhenHandler(Tuple* result) {
    std::cout << "Hello from CameraHandler" << std::endl;
    std::cout << "    Subject is " << result->subject << std::endl;

    auto outTuples = new std::vector<Tuple>();

    auto camName = result->subject.dat.str;
    auto pixelFormat = result->object.dat.u64;

    int fd = open(camName, O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error(
                std::format("ioctl failed with error: {0}", strerror(errno)));
    }

    struct v4l2_fmtdesc fmtdesc {
            .index = 0,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    };

//        outTuples->push_back(Tuple {
//            result->subject,
//            hasFormat,
//            TupleNoun::fromString(ss.str())
//        });

        struct v4l2_frmsizeenum framesize {
                .index = 0,
                .pixel_format = static_cast<uint32_t>(pixelFormat)
        };
        v4lEnumerate<struct v4l2_frmsizeenum, VIDIOC_ENUM_FRAMESIZES>(fd, framesize, [&result, &outTuples, &camName](auto desc) {
            if(desc.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                std::cout << "Discrete: " << desc.discrete.width << "x" << desc.discrete.height << std::endl;

                outTuples->push_back(Tuple {
                    result->subject,
                    hasWidth,
                    TupleNoun::fromUint(desc.discrete.width)
                });

                outTuples->push_back(Tuple {
                        result->subject,
                        hasHeight,
                        TupleNoun::fromUint(desc.discrete.height)
                });
            } else {
                throw std::runtime_error("Unimplemented framesize handler...");
            }
        });

    return outTuples;
}

extern "C" void free_tuple_obj(void* obj) {}

extern "C" void free_tuple_subj(void* subj) {}

