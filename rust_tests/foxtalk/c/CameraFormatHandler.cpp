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
        mk_tuple_noun_symbol("is a"),
        mk_tuple_noun_symbol("camera")
    );
}

std::vector<Tuple>* WhenHandler(Tuple result) {
    std::cout << "Hello from CameraFormatHandler" << std::endl;

    auto outTuples = new std::vector<Tuple>();

    auto subject = get_tuple_subject(result);
    auto camName = get_tuple_noun_as_symbol(subject);

    std::cout << "    running for camName: " << camName << std::endl;

    int fd = open(camName, O_NONBLOCK);
    if (fd < 0) {
        throw std::runtime_error(
                std::format("ioctl failed with error: {0}", strerror(errno)));
    }

    struct v4l2_fmtdesc fmtdesc {
            .index = 0,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    };

    v4lEnumerate<struct v4l2_fmtdesc, VIDIOC_ENUM_FMT>(fd, fmtdesc, [subject, &result, &outTuples](auto desc) {
        std::cout << "Enumd format [" << desc.index << "]: " << desc.description << std::endl;
        std::stringstream ss;
        ss << desc.description;

        outTuples->push_back(mk_tuple(
            subject,
            mk_tuple_noun_symbol("has format"),
            mk_tuple_noun_u64(desc.pixelformat)
        ));

        std::string s = ss.str();

        outTuples->push_back(mk_tuple(
            get_tuple_subject(result),
            mk_tuple_noun_symbol("has format name"),
            mk_tuple_noun_symbol(s.c_str())
        ));
    });

    return outTuples;
}