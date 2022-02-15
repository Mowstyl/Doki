"""Installation module."""

import platform

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

headers = ["src/doki/platform.h",
           "src/doki/funmatrix.h",
           "src/doki/qstate.h",
           "src/doki/qgate.h",
           "src/doki/arraylist.h",
           "src/doki/qops.h"]

sources = ["src/doki/platform.c",
           "src/doki/funmatrix.c",
           "src/doki/qstate.c",
           "src/doki/arraylist.c",
           "src/doki/qops.c",
           "src/doki/doki.c"]


ON_WINDOWS = platform.system() == "Windows"
_comp_args = [""]


class DokiBuild(build_ext):
    """Class with build attributes."""

    description = "Build Doki with custom build options"

    def build_extensions(self):
        """Add compiler specific arguments and prefixes."""
        compiler = self.compiler.compiler_type
        openmp_flag = "fopenmp"
        if ON_WINDOWS and compiler != 'mingw32':
            openmp_flag = "openmp"
        prefix = '-' if compiler != 'msvc' else '/'
        _comp_args[0] = prefix + openmp_flag
        build_ext.build_extensions(self)


def main():
    """Code to be executed on install."""
    setup(
        name="doki-Mowstyl",
        version="1.3.1",
        author="Hernán Indíbil de la Cruz Calvo",
        author_email="HernanIndibil.LaCruz@alu.uclm.es",
        cmdclass={'build_ext': DokiBuild},
        license="MIT",
        url="https://github.com/Mowstyl/Doki",
        description="Python interface for Doki (QSimov core)",
        long_description="Python module containing Doki, the core of QSimov" +
                         " quantum computer simulation platform. Written in" +
                         " C with OpenMP parallelism.",
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
        ext_modules=[Extension('doki', sources=sources,
                               extra_compile_args=_comp_args,
                               extra_link_args=_comp_args)],
        data_files=[('headers', headers)],
        python_requires=">=3.6",
    )


if __name__ == "__main__":
    main()
