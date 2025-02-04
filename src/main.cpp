#include "emulator.hpp"

int main (int argc, char *argv[]) {
    Emulator emu;

    emu.initGraphicsContext();

    auto romPath = std::filesystem::current_path() / (argc > 1 ? argv[1] : "Metroid Prime - Federation Force (Europe) (En,Fr,De,Es,It).3ds");
    if (!emu.loadROM(romPath)) {
        // For some reason just .c_str() doesn't show the proper path
        Helpers::panic("Failed to load ROM file: %s", romPath.string().c_str());
    }

    emu.run();
}