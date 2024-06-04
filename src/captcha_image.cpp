#include "captcha.hpp"
#include <png.h>
#include <jpeglib.h>
#include <stdexcept>

namespace captcha {
    captcha_image::captcha_image() {

    }

    captcha_image::captcha_image(uint32_t w, uint32_t h, uint32_t spacing) : width(w), height(h), spacing(spacing) {
        data.resize(w * h, 255);
        ok = true;
    }

    captcha_image::captcha_image(const char *source) {
        png_image image;

        auto file_data = read_file(source);
        if(file_data.size() == 0) {
            ok = false;
            return;
        }
        memset(&image, 0, (sizeof image));
        image.version = PNG_IMAGE_VERSION;
        if (png_image_begin_read_from_memory(&image, file_data.data(), file_data.size())) {
            image.format = PNG_FORMAT_GRAY;
            data.resize(PNG_IMAGE_SIZE(image));
            if (png_image_finish_read(&image, NULL, data.data(), 0, NULL)) {
                width = image.width;
                height = image.height;
                ok = true;              
            }
        }
    }

    std::string captcha_image::to_jpeg() const {
        if(!ok) {
            return "";
        }

        auto jpeg_error_exist = [](j_common_ptr cinfo) -> void
        {
            throw std::runtime_error("jpeg error");
        };

        unsigned char* output_buffer = NULL;
        jpeg_compress_struct cinfo;
        jpeg_error_mgr jerr;
        try {
            cinfo.err = jpeg_std_error(&jerr);
            jerr.error_exit = jpeg_error_exist;

            jpeg_create_compress(&cinfo);
            unsigned long output_size = 0;
            jpeg_mem_dest(&cinfo, &output_buffer, &output_size);
            cinfo.input_components = 1;
            cinfo.image_height = height;
            cinfo.image_width = width;
            cinfo.in_color_space = JCS_GRAYSCALE;

            jpeg_set_defaults(&cinfo);
            jpeg_start_compress(&cinfo, TRUE);

            // Read the input data line by line and write to the output buffer
            JSAMPROW row_pointer[1];
            uint32_t row_stride = width;
            uint32_t y = 0;
            while (y < height) {
                row_pointer[0] = (unsigned char*)(data.data() + y * width);
                jpeg_write_scanlines(&cinfo, row_pointer, 1);
                y++;
            }

            jpeg_finish_compress(&cinfo);
            jpeg_destroy_compress(&cinfo);

            // Convert the output buffer to a std::vector
            if(output_buffer) {
                auto str = std::string(output_buffer, output_buffer + output_size);
                free(output_buffer);
                return std::move(str);
            } else {
                return "";
            }
        } catch(std::exception& ex) {
            if(output_buffer) {
                free(output_buffer);
            }
            return "";
        }
    }
}