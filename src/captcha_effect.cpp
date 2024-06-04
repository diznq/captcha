#include "captcha.hpp"
#include <png.h>

namespace captcha {

    captcha_effect::captcha_effect() : ok(false) {}

    captcha_effect::captcha_effect(const char *source) {
        png_image image;

        auto file_data = read_file(source);
        memset(&image, 0, (sizeof image));
        image.version = PNG_IMAGE_VERSION;
        if (png_image_begin_read_from_memory(&image, file_data.data(), file_data.size())) {
            image.format = PNG_FORMAT_GRAY;
            data.resize(PNG_IMAGE_SIZE(image));
            if (png_image_finish_read(&image, NULL, data.data(), 0, NULL)) {
                width = image.width;
                height = image.height;
                channels = 4;
                ok = true;              
            }
        }
    }

}