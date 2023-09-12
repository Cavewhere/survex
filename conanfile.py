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
    ("wxwidgets/3.1.4@bincrafters/stable"),
    ("glew/2.2.0"),
    ("proj/9.1.0"),
    ("libtiff/4.5.0"),
    ("zlib/1.2.13")
    ]

    generators = "cmake_find_package", "cmake_paths"

    def configure(self):
        #This is survex dependancy
        self.options["wxwidgets"].webview=False
        self.options["proj"].shared=True
        self.options["zlib"].shared=True
        self.options["wxwidgets"].shared=True

