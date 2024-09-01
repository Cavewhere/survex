from conans import ConanFile, CMake, tools
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
    ("wxwidgets/[>=3.2.5]"),
    ("glew/2.2.0"),
    ("proj/9.2.1"),
    ("zlib/1.2.13"),
    ("libtiff/4.0.9"),
    ("gdal/[>=3.8.3]")
    ]

    generators = "cmake_find_package", "cmake_paths", "cmake"

    def configure(self):
        #This is survex dependancy
        #        self.options["wxwidgets"].webview=False
        self.options["wxwidgets"].shared=True
        self.options["proj"].shared=True
        self.options["proj"].with_tiff=False
        self.options["zlib"].shared=True

