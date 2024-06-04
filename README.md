![CAPTCHA](output.jpg?raw=true "Title")

## Simple CAPTCHA library for C++

Compile: `g++ -O2 src/*.cpp -ljpeg -lpng -o captcha`

Use: `./captcha HELLO`

As a library:

```cpp
#include "captcha.hpp"
#include <fstream>

int main(int argc, const char **argv) {
    // image containing the alphabet and actual letters, must be monospace
    captcha::captcha_generator gen("resources/captcha-alphabet.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    // distortion if any is to be applied, must be a normal map
    captcha::captcha_effect distortion("resources/captcha-distortion.png");
    // background image if any is to be applied, will be converted to grayscale when used
    captcha::captcha_image background("resources/captcha-background.png");

    auto image = gen.create(
        argc >= 2 ? argv[1] : "HELLOWORLD", 
        10, // spacing between letters
        &background,
        &distortion,
        4   // distortion dampening, the higher, the less effect distortion has
    );

    // save the result to a file
    std::ofstream fw("output.jpg", std::ios::binary);
    if(fw && fw.is_open()) {
        fw << image.to_jpeg();
    }
}
```