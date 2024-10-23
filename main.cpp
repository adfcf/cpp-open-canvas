#include <iostream>
#include "window.h"

int main() {

    oc::Window my_window{"my pretty window", 400, 300, 0.25f};

    while (my_window.is_alive()) {
        my_window.get_framebuffer().color_at(10, 10) = oc::Red;
        my_window.swap();
    }

    return 0;
}
