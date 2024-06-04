#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <functional>

namespace captcha {
    
    static std::vector<char> read_file(const char *file) {
        std::vector<char> out;
        FILE *f = fopen(file, "rb");
        fseek(f, 0, SEEK_END);
        size_t size = (size_t)ftell(f);
        out.resize(size);
        fseek(f, 0, SEEK_SET);
        fread(out.data(), size, 1, f);
        fclose(f);
        return out;
    }

    struct captcha_point2d {
        uint32_t x = 0;
        uint32_t y = 0;
    };

    struct captcha_image {
        bool ok = false;
        uint32_t width;
        uint32_t height;
        uint32_t spacing;
        std::vector<char> data;

        /// @brief Initialize an empty image
        captcha_image();

        /// @brief Initialize an empty white image
        /// @param w width
        /// @param h height
        /// @param spacing letter spacing
        captcha_image(uint32_t w, uint32_t h, uint32_t spacing = 0);

        /// @brief Initialize a grayscale image from a PNG
        /// @param source path to a PNG file
        captcha_image(const char *source);

        std::string to_jpeg() const;
    };

    struct captcha_effect {
        bool ok = false;
        uint32_t width;
        uint32_t height;
        uint8_t channels;
        std::vector<unsigned char> data;

        /// @brief Initialize an empty effect
        captcha_effect();

        /// @brief Initialize an effect from a PNG file
        /// @param source path to a PNG file
        captcha_effect(const char *source);
    };

    class captcha_generator {
        bool ok = false;
        std::string alphabet;
        char lookup[128];
        char rnd[512];
        size_t alphabet_size;
        uint32_t width;
        uint32_t height;
        uint32_t x_unit;
        uint32_t y_unit;
        std::vector<char> data;
        std::function<void(char*, size_t)> randomness;
    public:
        /// @brief Initialize an empty CAPTCHA genertor
        captcha_generator();

        /// @brief Initialize the CAPTCHA generator
        /// @param source path to the alphabet
        /// @param alphabet_ alphabet letters
        /// @param randomness_source_ randomness source
        captcha_generator(
            const char *source, 
            const std::string& alphabet_,
            std::function<void(char*, size_t)> randomness_source_ = nullptr
        );

        ///@brief Write text into the image
        /// @param image image to be modified
        /// @param text text to be written
        /// @param noise noise, must be at least 512 bytes big
        /// @return int 0 if ok, <0 if error
        int write(captcha_image& image, const std::string& text, unsigned char *noise);

        /// @brief Create a new captcha image
        /// @param text CAPTCHA text
        /// @param spacing letter spacing
        /// @param background background image
        /// @param distortion distortion normal map
        /// @param distortion_dampening distortion dampening
        /// @param background_shift pixel shift of the background
        /// @param distortion_shift pixel shift of the distortion
        /// @param noise random noise
        /// @return CAPTCHA image
        captcha_image create(
            const std::string& text,
            uint32_t spacing = 10,
            const captcha_image* background = NULL,
            const captcha_effect* distortion = NULL,
            const unsigned distortion_dampening = 5,
            const captcha_point2d background_shift = captcha_point2d{},
            const captcha_point2d distortion_shift = captcha_point2d{},
            unsigned char *noise = NULL
        );
    };
}