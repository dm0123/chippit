from conan import ConanFile

class Chip8(ConanFile):
    name = "chip8"
    version = "0.0.1"
    author = "Dmitrii Polomin dpolomin@gmail.com"
    settings = "os", "compiler", "build_type", "arch"

    generators = "CMakeDeps"

    def requirements(self):
        self.requires("sdl/[^2.0.0]")