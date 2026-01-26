from conan import ConanFile
import os
from conan.tools.files import copy
from pathlib import Path
import shutil

class Magpie(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("libnghttp2/1.66.0")
        # TODO: sync with whatever ubuntu uses
        self.requires("openssl/3.6.0")
        self.requires("asio/1.36.0")
        # Unused, will be used Later:tm:
        self.requires("zlib/1.3.1")

        self.test_requires(
            "libcurl/8.17.0"
        )
        self.test_requires(
            "cpr/1.14.1"
        )

    def configure(self):
        # Conan's version of libcurl default-disables like all the protocols,
        # including http2. For obvious reasons, this is a tiny bit of a problem
        # when trying to use a libcurl-extension to test an HTTP/2 server
        self.options["libcurl"].with_nghttp2 = True
