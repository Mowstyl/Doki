"""Installation module.
Based on https://github.com/cudamat/cudamat/blob/master/setup.py"""

import distutils.cygwinccompiler
import os
# import sys

# from distutils.spawn import spawn, find_executable
from distutils.sysconfig import get_python_inc
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


# on Windows, we need the original PATH without Anaconda's compiler in it:
PATH = os.environ.get('PATH')
# _comp_args = ["-Xcompiler", "-openmp"]
_comp_args = ["-fopenmp"]
# _link_args = ["-lgomp"]
_link_args = ["-fopenmp"]


class DokiBuild(build_ext):
    """Class with build attributes."""
    def build_extensions(self):
        """Add compiler specific arguments and prefixes."""
        # Remove the msvc version check since it always raises an exception
        distutils.cygwinccompiler.get_msvcr = lambda: []
        self.compiler = distutils.cygwinccompiler.CygwinCCompiler()
        # Remove the -mcygwin flag since it has been removed
        self.compiler.set_executable('compiler', 'gcc -O -Wall')
        self.compiler.set_executable('compiler_so', 'gcc -mdll -O -Wall')
        self.compiler.set_executable('compiler_cxx', 'g++ -O -Wall')
        self.compiler.set_executable('linker_exe', 'gcc')
        self.compiler.set_executable('linker_so', 'gcc -shared')
        # Add the library dir
        # python_libs = os.path.join(distutils.sysconfig.PREFIX, "libs")
        # self.compiler.add_library_dir(python_libs)
        # self.compiler.add_runtime_library_dir(python_libs)
        '''
        self.compiler.src_extensions.append('.cu')
        self.compiler.set_executable('compiler_so', 'nvcc')
        self.compiler.set_executable('linker_so', 'nvcc --shared')
        if hasattr(self.compiler, '_c_extensions'):
            self.compiler._c_extensions.append('.cu')  # needed for Windows
        self.compiler.spawn = self.spawn
        '''
        build_ext.build_extensions(self)

    '''
    def spawn(self, cmd, search_path=1, verbose=0, dry_run=0):
        """
        Perform any CUDA specific customizations before actually launching
        compile/link etc. commands.
        """
        if (sys.platform == 'darwin' and len(cmd) >= 2 and cmd[0] == 'nvcc' and
                cmd[1] == '--shared' and cmd.count('-arch') > 0):
            # Versions of distutils on OSX earlier than 2.7.9 inject
            # '-arch x86_64' which we need to strip while using nvcc for
            # linking
            while True:
                try:
                    index = cmd.index('-arch')
                    del cmd[index:index+2]
                except ValueError:
                    break
        elif self.compiler.compiler_type == 'msvc':
            # There are several things we need to do to change the commands
            # issued by MSVCCompiler into one that works with nvcc. In the end,
            # it might have been easier to write our own CCompiler class for
            # nvcc, as we're only interested in creating a shared library to
            # load with ctypes, not in creating an importable Python extension.
            # - First, we replace the cl.exe or link.exe call with an nvcc
            #   call. In case we're running Anaconda, we search cl.exe in the
            #   original search path we captured further above -- Anaconda
            #   inserts a MSVC version into PATH that is too old for nvcc.
            cl_exec = find_executable("cl.exe", PATH)
            if cl_exec is not None:
                cl_exec = os.path.dirname(cl_exec)
            cmd[:1] = ['nvcc', '--compiler-bindir',
                       cl_exec or cmd[0]]
            # - Secondly, we fix a bunch of command line arguments.
            for idx, c in enumerate(cmd):
                # create .dll instead of .pyd files
                if '.pyd' in c:
                    cmd[idx] = c = c.replace('.pyd', '.dll')
                # replace /c by -c
                if c == '/c':
                    cmd[idx] = '-c'
                # replace /DLL by --shared
                elif c == '/DLL':
                    cmd[idx] = '--shared'
                # remove --compiler-options=-fPIC
                elif '-fPIC' in c:
                    del cmd[idx]
                # replace /Tc... by ...
                elif c.startswith('/Tc'):
                    cmd[idx] = c[3:]
                # replace /Fo... by -o ...
                elif c.startswith('/Fo'):
                    cmd[idx:idx+1] = ['-o', c[3:]]
                # replace /LIBPATH:... by -L...
                elif c.startswith('/LIBPATH:'):
                    cmd[idx] = '-L' + c[9:]
                # replace /OUT:... by -o ...
                elif c.startswith('/OUT:'):
                    cmd[idx:idx+1] = ['-o', c[5:]]
                # remove /EXPORT:initlibcudamat or /EXPORT:initlibcudalearn
                elif c.startswith('/EXPORT:'):
                    del cmd[idx]
                # replace cublas.lib by -lcublas
                elif c == 'cublas.lib':
                    cmd[idx] = '-lcublas'
            # - Finally, we pass on all arguments starting with a '/' to the
            #   compiler or linker, and have nvcc handle all other arguments
            if '--shared' in cmd:
                pass_on = '--linker-options='
                # we only need MSVCRT for a .dll, remove CMT if it sneaks in:
                cmd.append('/NODEFAULTLIB:libcmt.lib')
            else:
                pass_on = '--compiler-options='
            cmd = ([c for c in cmd if c[0] != '/'] +
                   [pass_on + ','.join(c for c in cmd if c[0] == '/')])
            # For the future: Apart from the wrongly set PATH by Anaconda, it
            # would suffice to run the following for compilation on Windows:
            # nvcc -c -O -o <file>.obj <file>.cu
            # And the following for linking:
            # nvcc --shared -o <file>.dll <file1>.obj <file2>.obj -lcublas
            # This could be done by a NVCCCompiler class for all platforms.
        spawn(cmd, search_path, verbose, dry_run)
    '''


def main():
    """Code to be executed on install."""
    python_includes = get_python_inc()
    python_libs = os.path.join(os.path.dirname(python_includes), "libs")
    setup(
        name="doki_gpu-Mowstyl",
        version="1.3.0",
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
        ext_modules=[Extension('doki_gpu', sources=sources,
                               include_dirs=[python_includes],
                               library_dirs=[python_libs],
                               extra_compile_args=_comp_args,
                               extra_link_args=_link_args)],
        data_files=[('headers', headers)],
        python_requires=">=3.6",
    )


if __name__ == "__main__":
    main()
