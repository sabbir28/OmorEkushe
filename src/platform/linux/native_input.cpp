#include "platform/linux/native_input.h"

namespace bijoy::platform::linux {

    void DoKeyboard(int flags, int scanCode) {
        (void)flags;
        (void)scanCode;
    }

    void DoKeyboardVk(int flags, int vk) {
        (void)flags;
        (void)vk;
    }

} // namespace bijoy::platform::linux
