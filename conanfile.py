from conan import ConanFile
import os
from conan.tools.files import copy
from pathlib import Path
import shutil

class NyaaTheUntitledGame(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libnghttp2/1.66.0")
        # TODO: sync with whatever ubuntu uses
        self.requires("openssl/3.6.0")
        self.requires("asio/1.36.0")
        # Unused, will be used Later:tm:
        self.requires("zlib/1.3.1")
