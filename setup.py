"""Installation module."""

from setuptools import setup, Extension

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


def main():
    """Code to be executed on install."""
    setup(
        name="doki-Mowstyl",
        version="1.0.0",
        author="Hernán Indíbil de la Cruz Calvo",
        author_email="HernanIndibil.LaCruz@alu.uclm.es",
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
                               extra_compile_args=["-openmp"])],
        data_files=[('headers', headers)],
        python_requires=">=3.6",
    )


if __name__ == "__main__":
    main()
