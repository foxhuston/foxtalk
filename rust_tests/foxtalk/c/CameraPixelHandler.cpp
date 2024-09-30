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

#include "reactor.h"

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
    return mk_tuple(
        mk_tuple_noun_query(),
        mk_tuple_noun_symbol("has format"),
        mk_tuple_noun_query()
    );
}

std::vector<Tuple>* WhenHandler(Tuple* result) {
    std::cout << "Hello from CameraPixelHandler" << std::endl;

    auto subject = get_tuple_subject(result);
    auto object = get_tuple_object(result);

    auto outTuples = new std::vector<Tuple>();

    auto camName = get_tuple_noun_as_symbol(subject);
    auto pixelFormat = get_tuple_noun_as_u64(object);

    std::cout << "    running for camName: " << camName << " and pixel format " << pixelFormat << std::endl;

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
        v4lEnumerate<struct v4l2_frmsizeenum, VIDIOC_ENUM_FRAMESIZES>(fd, framesize, [&result, &outTuples, &object](auto desc) {
            if(desc.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
                std::cout << "Discrete: " << desc.discrete.width << "x" << desc.discrete.height << std::endl;

                outTuples->push_back(mk_tuple(
                    object,
                    mk_tuple_noun_symbol("has width"),
                    mk_tuple_noun_u64(desc.discrete.width)
                ));

                outTuples->push_back(mk_tuple(
                        object,
                        mk_tuple_noun_symbol("has height"),
                        mk_tuple_noun_u64(desc.discrete.height)
                ));
            } else {
                throw std::runtime_error("Unimplemented framesize handler...");
            }
        });

    return outTuples;
}

extern "C" void free_tuple_obj(void* obj) {}

extern "C" void free_tuple_subj(void* subj) {}

