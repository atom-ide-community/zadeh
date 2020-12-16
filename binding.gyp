# Note: C++ standard is repeated in configurations multiple times for different configurations
{
  "targets": [{
      "target_name": "fuzzaldrinplusfast",
      "sources": [ "src/common.h", "src/fuzzaldrin.cc", "src/scorer.cc", "src/path_scorer.cc", "src/filter.cc", "src/query.cc", "src/matcher.cc",  "src/tree.h" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "VendorLib"
      ],
      # Compiler flags:
      'default_configuration': 'Release',
      'configurations': {
        # Release Settings
        'Release': {
          'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS', 'NDEBUG' ],
          "cflags": [ "-fno-exceptions", "-Ofast" ],
          "cflags_cc": [ "-fno-exceptions", "-Ofast", "-std=c++17" ],
          "xcode_settings": {
            'GCC_OPTIMIZATION_LEVEL': '3', # stop gyp from defaulting to -Os
            "CLANG_CXX_LIBRARY": "libc++",
            "CLANG_CXX_LANGUAGE_STANDARD":"c++17",
            'MACOSX_DEPLOYMENT_TARGET': '10.15'
          },
          "msvs_settings": {
            "VCCLCompilerTool": {
              'ExceptionHandling': 0,               # /EHsc
              'MultiProcessorCompilation': 'true',
              'StringPooling': 'true',              # pool string literals
              'RuntimeTypeInfo': 'false',
              'Optimization': 4,              # full optimizations /O2  ==	/Og /Oi /Ot /Oy /Ob2 /GF /Gy
              'InlineFunctionExpansion': 3,   # stop gyp from defaulting to /Ob2
              "AdditionalOptions": [
                # C++ standard
                "/std:c++latest",

                # Optimizations
                "/Ob3",  # agressive inline
                "/GL",   # whole Program Optimization  # /LTCG is implied with /GL.
                "/DNDEBUG"  # turn off asserts
              ],
            }
          }
        },
        # Debug Settings
        'Debug': {
          'defines': [ 'DEBUG', 'NAPI_CPP_EXCEPTIONS' ],
          'cflags': [ '-g', '-O0', '-ldl' ],
          "cflags_cc": [
            '-g',
            '-ldl',
            '-fexceptions', # enable exceptions

            # C++ standard
            "-std=c++17",
            "-stdlib=libc++",

            # Clang warnings
            "-Wall",
            "-Wextra", # reasonable and standard
            "-Wshadow", # warn the user if a variable declaration shadows one from a parent context
            "-Wnon-virtual-dtor", # warn the user if a class with virtual functions has a non-virtual destructor. This helps catch hard to track down memory errors
            "-Wold-style-cast", # warn for c-style casts
            "-Wcast-align", # warn for potential performance problem casts
            "-Wunused", # warn on anything being unused
            "-Woverloaded-virtual", # warn if you overload (not override) a virtual function
            "-Wpedantic", # warn if non-standard C++ is used
            "-Wconversion", # warn on type conversions that may lose data
            "-Wsign-conversion", # warn on sign conversions
            "-Wnull-dereference", # warn if a null dereference is detected
            "-Wdouble-promotion", # warn if float is implicit promoted to double
            "-Wformat=2", # warn on security issues around functions that format output (ie printf)

            # GCC Warnings
            "-Wmisleading-indentation", # warn if indentation implies blocks where blocks do not exist
            "-Wduplicated-cond", # warn if if / else chain has duplicated conditions
            "-Wduplicated-branches", # warn if if / else branches have duplicated code
            "-Wlogical-op", # warn about logical operations being used where bitwise were probably wanted
            "-Wuseless-cast", # warn if you perform a cast to the same type
           ], # GCC/CLANG
          "msvs_settings": {
            "VCCLCompilerTool": {
              'ExceptionHandling': 2,       # /EHsc
              'BasicRuntimeChecks': 3,        # /RTC1
              'MinimalRebuild': 'false',
              'Optimization': 0,              # /Od /Oy-, no optimization
              "AdditionalOptions": [
                "/DEBUG",

                # C++ standard
                "/std:c++latest",
                # MSVC Warnings
                 # disable warnings on external headers
                "/experimental:external",
                "/external:W0",
                "/external:I <!@(node -p \"require('node-addon-api').include\")",
                "/external:anglebrackets",
                "/Wall", # all warnings,
                "/wd5044", # include path missing
                "/wd4710", # function not inlined
                "/wd4514", # unreferenced inline function has been removed
                "/wd5045", # spectre mitigation insertion warning
                "/wd4820", # 'bytes' bytes padding added after construct 'member_name'
                "/wd4626", # assignment operator was implicitly defined as deleted
                "/wd5027"  # move operator was implicitly defined as deleted
              ],
            },
          }, # MSVC
          "xcode_settings": {
            'GCC_OPTIMIZATION_LEVEL': '0', # stop gyp from defaulting to -Os
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            "CLANG_CXX_LIBRARY": "libc++",
            "CLANG_CXX_LANGUAGE_STANDARD":"c++17",
            'MACOSX_DEPLOYMENT_TARGET': '10.15'
          } # XCODE
        }, # Debug
      }, # configurations
  }] # targets
}
