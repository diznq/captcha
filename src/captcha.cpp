#include "captcha.hpp"
#include <time.h>
#include <png.h>

namespace captcha {
    captcha_generator::captcha_generator() : ok(false) {

    }

    captcha_generator::captcha_generator(
        const char *source,
        const std::string& alphabet_,
        std::function<void(char*, size_t)> randomness_
    ) : alphabet(alphabet_), randomness(randomness_) {
        // initialize random noise and load the alphabet
        size_t i;
        png_image image;

        if(randomness) {
            randomness(rnd, sizeof(rnd));
        } else {
            srand(time(NULL));
            for(i = 0; i < sizeof(rnd); i++)
                rnd[i] = rand() & 255;
        }
        alphabet_size = alphabet.length();

        memset(lookup, 0, sizeof(lookup));

        for(i = 0; i < alphabet_size; i++) {
            lookup[alphabet[i]] = i;
        }

        auto file_data = read_file(source);
        memset(&image, 0, (sizeof image));
        image.version = PNG_IMAGE_VERSION;
        if (png_image_begin_read_from_memory(&image, file_data.data(), file_data.size())) {
            image.format = PNG_FORMAT_GRAY;
            data.resize(PNG_IMAGE_SIZE(image));
            if (png_image_finish_read(&image, NULL, data.data(), 0, NULL)) {
                width = image.width;
                height = image.height;
                x_unit = width / alphabet_size;
                y_unit = height;
                ok = true;
            }
        }
    }

    int captcha_generator::write(captcha_image& image, const std::string& text, unsigned char *noise) {
        if(!ok || !image.ok) return -1;
        size_t text_size = text.length();
        size_t i, p, y, x;
        const size_t iw = image.width, ih = image.height, spacing = image.spacing;

        // copy the letters into the image
        for(i = 0; i < text_size; i++) {
            p = lookup[text[i]] * x_unit;
            for(y = 0; y < y_unit; y++) {
                for(x = 0; x < x_unit; x++) {
                    image.data[
                        y * iw + x + (spacing + i * (x_unit + spacing))
                    ] &= data[y * width + p + x];
                }
            }
        }

        // apply noise on top of the image
        if(!noise) noise = (unsigned char*)rnd;
        for(y = 0, i = 0; y < image.height; y++) {
            for(x = 0; x < image.width; x++, i++) {
                image.data[i] ^= rnd[i & 511] & 0x53;
            }
        }
        
        return 0;
    }

    captcha_image captcha_generator::create(
        const std::string& text,
        uint32_t spacing,
        const captcha_image* background,
        const captcha_effect* effect,
        const unsigned int distortion_dampening,
        const captcha_point2d background_shift,
        const captcha_point2d effect_shift,
        unsigned char *noise
    ) {
        char *copy;
        unsigned char *rg;
        const size_t text_size = text.length();
        size_t y = 0, x = 0, ty = 0, tx = 0, i = 0;
        uint32_t iw = spacing + (x_unit + spacing) * text.length();
        uint32_t ih = y_unit;
        if(!ok) return {};

        captcha_image image(iw, ih, spacing);

        if(background && background->ok) {
            for(y = 0, i = 0; y < ih; y++) {
                for(x = 0; x < iw; x++, i++) {
                    image.data[i] = background->data[
                        ((y + background_shift.y) % background->height) * background->width
                        + (x + background_shift.x) % background->width
                    ];
                }
            }
        }

        write(image, text, noise);

        if(effect && effect->ok) {
            std::vector<char> copy = image.data;
            for(y = 0, i = 0; y < ih; y++) {
                for(x = 0; x < iw; x++, i++) {
                    rg = (unsigned char*)(effect->data.data() + (
                        ((y + effect_shift.y) % effect->height) * effect->width
                        + (x + effect_shift.x) % effect->width
                    ));

                    int32_t x_shift = (int32_t)(rg[0]) - 127;
                    int32_t y_shift = (int32_t)(rg[1]) - 127;

                    x_shift /= distortion_dampening;
                    y_shift /= distortion_dampening;
                    
                    tx = (x + (x_shift)) % iw;
                    ty = (y + (y_shift)) % ih;

                    copy[i] = image.data[ty * iw + tx];
                }
            }
            image.data = std::move(copy);
        }

        return image;
    }
}