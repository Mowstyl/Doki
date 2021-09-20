"""Installation module."""

import platform
from setuptools import setup, find_packages, Extension
from setuptools.command.build_ext import build_ext

ON_WINDOWS = platform.system() == "Windows"
_comp_args = ["DSHARED=1"]
sources = ["src/doki/platform.c",
           "src/doki/funmatrix.c",
           "src/doki/qstate.c",
           "src/doki/arraylist.c",
           "src/doki/qops.c",
           "src/doki/doki.c"]


class DokiBuild(build_ext):
    """Class with build attributes."""

    description = "Build Doki with custom build options"
    user_options = build_ext.user_options + [
        ('gcov', None, "Enable GCC code coverage collection"),
        ('vector', None, "Include the vector_XXX() functions;"
         "they are unstable and under active development"),
        ('static', None, "Enable static linking compile time options."),
        ('static-dir=', None, "Enable static linking and specify location."),
        ('gdb', None, "Build with debug symbols."),
    ]

    def initialize_options(self):
        """Set default values for options."""
        build_ext.initialize_options(self)
        self.gcov = False
        self.vector = False
        self.static = False
        self.static_dir = False
        self.gdb = False

    def finalize_options(self):
        """Modify compiler arguments based on specified options."""
        build_ext.finalize_options(self)
        if self.gcov:
            if ON_WINDOWS:
                raise ValueError("Cannot enable GCC code coverage on Windows")
            _comp_args.append('DGCOV=1')
            _comp_args.append('O0')
            _comp_args.append('-coverage')
            self.libraries.append('gcov')
        if self.vector:
            _comp_args.append('DVECTOR=1')
        if self.static:
            _comp_args.remove('DSHARED=1')
            _comp_args.append('DSTATIC=1')
        if self.gdb:
            _comp_args.append('ggdb')
        if self.static_dir:
            _comp_args.remove('DSHARED=1')
            _comp_args.append('DSTATIC=1')
            self.include_dirs.append(self.static_dir + '/include')
            self.library_dirs.append(self.static_dir + '/lib')

    def build_extensions(self):
        """Add compiler specific arguments and prefixes."""
        compiler = self.compiler.compiler_type
        if compiler == 'mingw32':
            _comp_args.append('DMSYS2=1')
        elif ON_WINDOWS and not self.static:
            # MSVC shared build
            _comp_args.append('DMSC_USE_DLL')
        _prefix = '-' if compiler != 'msvc' else '/'
        for i in range(len(_comp_args)):
            _comp_args[i] = ''.join([_prefix, _comp_args[i]])
        build_ext.build_extensions(self)


extensions = [
    Extension('doki',
              sources=sources,
              extra_compile_args=_comp_args,
              )
]

cmdclass = {'build_ext': DokiBuild}


def main():
    """Code to be executed on install."""
    setup(
        name="doki-Mowstyl",
        version="1.0.0",
        author="Hernán Indíbil de la Cruz Calvo",
        author_email="HernanIndibil.LaCruz@alu.uclm.es",
        cmdclass=cmdclass,
        license="MIT",
        url="https://github.com/Mowstyl/QSimov",
        description="Python interface for Doki (QSimov core)",
        long_description="TODO",
        zip_safe=False,
        include_package_data=True,
        package_data={'doki': [
            '*.pxd',
        ]},
        packages=find_packages(),
        classifiers=[
            "Intended Audience :: Developers",
            "Intended Audience :: Science/Research",
            "Operating System :: Microsoft :: Windows",
            "Operating System :: POSIX :: Linux",
            "Operating System :: MacOS",
            'Programming Language :: C',
            'Programming Language :: Python :: 3 :: Only',
            'Programming Language :: Python :: 3.6',
            'Programming Language :: Python :: 3.7',
            'Programming Language :: Python :: 3.8',
            'Programming Language :: Python :: Implementation :: CPython',
            'Topic :: Software Development :: Libraries :: Python Modules',
            "Topic :: Scientific/Engineering",
        ],
        keywords="qsimov simulator quantum",
        ext_modules=extensions,
        python_requires=">=3.6",
    )


if __name__ == "__main__":
    main()
