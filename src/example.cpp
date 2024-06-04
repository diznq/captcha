#include "captcha.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

int main(int argc, const char **argv) {
    captcha::captcha_generator gen("resources/captcha-alphabet.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    captcha::captcha_effect distortion("resources/captcha-distortion.png");
    captcha::captcha_image background("resources/captcha-background.png");

    auto start = std::chrono::high_resolution_clock::now();
    auto image = gen.create(
        argc >= 2 ? argv[1] : "HELLOWORLD", 
        10, 
        &background,
        &distortion,
        4
    );
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
    std::cout << "generated CAPTCHA in " << duration << " microseconds" << std::endl;
    std::ofstream fw("output.jpg", std::ios::binary);
    if(fw && fw.is_open()) {
        fw << image.to_jpeg();
    }
}