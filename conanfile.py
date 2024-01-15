from conan import ConanFile


class Project(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def configure(self):
        # We can control the options of our dependencies based on current options
        self.options["catch2"].with_main = True
        self.options["catch2"].with_benchmark = True
        self.options["boost"].header_only = True


    def requirements(self):
        self.requires("catch2/2.13.9")
        self.requires("pipes/1.0.0")
        self.requires("range-v3/0.12.0")
        self.requires("fmt/9.1.0")
        self.requires("confu_json/1.0.0")
        self.requires("boost/1.83.0", force=True)







    # def build_requirements(self):
    #     self.tool_requires("cmake/3.22.6")