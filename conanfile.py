from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps
import os, sys

class SurvexConan(ConanFile):
    name = "Survex"
    license = "GPLv3"
    author = "Olly olly@survex.com"
    url = "https://github.com/ojwb/survex"
    description = "Survex is very powerful cave survey software"
    topics = ("gis")
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    requires = [
    ("wxwidgets/[>=3.2.6]"),
    ("glew/2.2.0"),
    ("proj/9.3.1"),
    # ("zlib/1.2.13"),
    ("libtiff/[>=4.5.1]"),
    ("gdal/[>=3.5.3]")
    ]

    def requirements(self):
        self.requires("expat/[>=2.6.2]", override=True)
        self.requires("libpng/[>=1.6.44]", override=True)

    generators = "CMakeDeps", "CMakeToolchain", "VirtualBuildEnv", "VirtualRunEnv"

    def configure(self):
        #This is survex dependancy
        #        self.options["wxwidgets"].webview=False
        self.options["wxwidgets"].shared=True
        self.options["proj"].shared=True
        self.options["proj"].with_tiff=False
        # self.options["zlib"].shared=True
        self.options["libtiff"].zstd=False
        self.options["gdal"].with_arrow = False
        self.options["gdal"].with_curl = False
        self.options["gdal"].with_libiconv = False
        self.options["proj"].with_curl = False

