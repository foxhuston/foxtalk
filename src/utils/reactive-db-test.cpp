//
// Created by fox on 9/26/24.
//
#include <iostream>

#include <string.h> // this is where strerror lives, apparently.
#include <fcntl.h>
#include <functional>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

//#include "../reactive_db/ReactiveDb.h"
//#include "../reactive_db/Symbol.h"
#include "../reactive_db/Reactor.h"

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

//void addCameraDevice(ReactiveDb& db, string camName) {
//    int fd = open(camName.c_str(), O_NONBLOCK);
//    if (fd < 0) {
//        throw std::runtime_error(std::format("ioctl failed with error: {0}", strerror(errno)));
//    }
//
//    db.claim(camName, "is a", "camera");
//
//    struct v4l2_fmtdesc fmtdesc {
//            .index = 0,
//            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE
//    };
//    v4lEnumerate<struct v4l2_fmtdesc, VIDIOC_ENUM_FMT>(fd, fmtdesc, [&db, &camName, fd](auto desc) {
//        std::cout << "Enum'd format [" << desc.index << "]: " << desc.description << std::endl;
//
//        db.claim(desc.description, "is a", "format");
//        db.claim(desc.description, "has the id", (void*)desc.pixelformat); // omg.
//        db.claim(camName, "has the format", desc.description);
//
//        struct v4l2_frmsizeenum framesize {
//                .index = 0,
//                .pixel_format = desc.pixelformat
//        };
//        v4lEnumerate<struct v4l2_frmsizeenum, VIDIOC_ENUM_FRAMESIZES>(fd, framesize, [&db, &camName](auto desc) {
//            if(desc.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
//                // Hm. How do I say "has resolution in pixel format...?"
//                // Also, I'd like this to be un-nested. The presence of a camera should trigger
//                // the finding of pixel formats; the presence of pixel formats should find
//                db.claim(camName, "has resolution", new std::pair { desc.discrete.width, desc.discrete.height });
//
//                std::cout << "    Discrete: " << desc.discrete.width << "x" << desc.discrete.height << std::endl;
//            } else {
//                throw std::runtime_error("Unimplemented framesize handler...");
//            }
//        });
//    });
//
//}

int main() {
    Reactor db{};

    auto fox = db.symbol("fox");
    auto lexi = db.symbol("lexi");
    auto isA = db.symbol("is a");
    auto demonfox = db.symbol("demon fox");
    auto husky = db.symbol("husky");

    auto isHighlighted = db.symbol("is highlighted");
    auto blue = db.symbol("blue");

    db.claim({lexi, isA, husky});
    db.claim({fox, isA, demonfox});

    // When (you) are a husky:
    db.when({nullptr, isA, husky},
            [isHighlighted, blue](wish wish, size_t nBindings, Tuple* bindings) {
                if (nBindings == 1) {
                    std::cout << "Found a husky: " << bindings->subject << std::endl;
                    // Wish (you) were highlighted blue.
                    wish({ bindings->subject, isHighlighted, blue });
                } else {
                    throw std::runtime_error(
                            std::format("Unexpected number of bindings! Wanted 1, got {0}", nBindings));
                }
            });

    // Wish (you) are highlighted (color):
    db.when({ nullptr, isHighlighted, nullptr },
            [](wish wish, size_t nBindings, Tuple* bindings) {
                if (nBindings == 1) {
                    std::cout << "Will highlight " << bindings->subject << " " << bindings->object << std::endl;
                } else {
                    throw std::runtime_error(
                            std::format("Unexpected number of bindings! Wanted 1, got {0}", nBindings));
                }
    });

    db.tick();

//    // When (you) is a husky...
//    auto allTheHuskies = db.query(nullopt, "is a", "husky");
//    for (const auto &[who, isA, hoosk]: allTheHuskies) {
//        // (you) are highlighted blue
//        db.claim(who, "is highlighted", "blue");
//    }
//
//    // When (you) are highlighted (a color)...
//    auto highlights = db.query(nullopt, "is highlighted", nullopt);
//    for (const auto &[who, _, color]: highlights) {
//        std::cout << "Going to highlight "
//                  << *static_cast<Symbol *>(who)
//                  << " " << *static_cast<Symbol *>(color)
//                  << std::endl;
//    }
}
