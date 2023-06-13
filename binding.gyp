# https://github.com/nodejs/node-gyp/blob/main/addon.gypi
# https://github.com/nodejs/node/blob/eaaee92d9b0be82d7f40b2abb67f30ce525d4bc4/common.gypi
# https://github.com/chromium/gyp/blob/md-pages/docs/InputFormatReference.md

{
    'includes': [ 'common.gypi' ], # brings in a default set of options that are inherited from gyp

    "targets": [
        {
            "target_name": "<(target_name)",
            'type': 'loadable_module',
            "sources": [
                'src/cpp/addon.cc',
                'src/cpp/nwrfcsdk.cc',
                'src/cpp/Client.cc',
                'src/cpp/Pool.cc',
                'src/cpp/Server.cc',
                'src/cpp/Throughput.cc'
            ],
            'defines': [
                '_CONSOLE',
                'SAPwithUNICODE', 'UNICODE', '_UNICODE',
                '_FILE_OFFSET_BITS=64',
                '_LARGEFILE_SOURCE',
                'NDEBUG',
                'SAPwithTHREADS',
                'NAPI_CPP_EXCEPTIONS',
                'NAPI_VERSION=8',
                'sapnwrfc_EXPORTS'
            ],
            'include_dirs': ['<(napi_include_dir)/', '<(nwrfcsdk_dir)/include'],
            'conditions': [
                ['OS=="mac"',
                    {
                        'cflags+': ['-fvisibility=hidden'],
                        'cflags_cc!': [
                            '-Wpedantic', '-Wall', '-Wextra', '-Werror', '-Wnocompound-token-split-by-macro'
                            '-stdlib=libc++',
                            '-std=c++17',
                            '-Wc++17-extensions',
                            '-mmacosx-version-min=10.10',
                            '-fvisibility=hidden',
                            '-fPIC',
                            '-MD', '-MT'
                        ],
                        'defines': [
                            '_DARWIN_USE_64_BIT_INODE=1',
                            'SAPonDARW',
                            '__NO_MATH_INLINES',
                        ],
                        'link_settings': {
                            'library_dirs': [
                                '<(nwrfcsdk_dir)/lib'
                            ],
                            'libraries': [
                                '-lsapnwrfc',
                                '-lsapucum',
                                '-Wl,-rpath,<(nwrfcsdk_dir)/lib'
                            ],
                        },
                        'xcode_settings': {
                            'DYLIB_INSTALL_NAME_BASE': '@rpath',
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                            'CLANG_CXX_LIBRARY': 'libc++',
                            'MACOSX_DEPLOYMENT_TARGET': '10.10',
                            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
                        },
                    },
                ],
                [
                    'OS=="linux"',
                    {
                        # 'cflags_cc!': [
                        #     # '-Wall',
                        #     '-std=c++14',
                        #     '-Wno-unused-variable'
                        #     ],
                        'cflags_cc!': [
                            '-Wpedantic', '-Wall', '-Wextra', '-Werror', '-Wnocompound-token-split-by-macro'
                            '-stdlib=libc++',
                            '-std=c++17',
                            '-Wc++17-extensions',
                            '-mmacosx-version-min=10.10',
                            '-fvisibility=hidden',
                            '-fPIC',
                            '-MD', '-MT'
                        ],
                        'defines': [
                            'SAPonLIN'
                            'SAPonUNIX',
                        ],
                        'link_settings': {
                            'library_dirs': [
                                '<(nwrfcsdk_dir)/lib'
                            ],
                            'libraries': [
                                '-lsapnwrfc',
                                '-lsapucum',
                                '-Wl,-rpath,<(nwrfcsdk_dir)/lib'
                            ],
                        },
                    },
                ],
                [
                    'OS=="win"',
                    {
                        'type': 'shared_library',
                        'defines': [
                            'PLATFORM="win32"',
                            'WIN32',
                            '_AFXDLL',
                            '_CRT_NON_CONFORMING_SWPRINTFS',
                            '_CRT_SECURE_NO_DEPRECATE',
                            '_CRT_NONSTDC_NO_DEPRECATE',
                            'SAPonNT',
                            'UNICODE',
                            '_UNICODE',
                        ],
                        # 'msvs_configuration_attributes': {
                        #     'OutputDirectory': '$(SolutionDir)$(ConfigurationName)',
                        #     'IntermediateDirectory': '$(OutDir)\\obj',
                        # },
                        'msvs_settings': {
                            'VCCLCompilerTool': {
                                'WholeProgramOptimization': 'true' # /GL, whole program optimization, needed for LTCG
                            },
                            'VCLibrarianTool': {
                                'AdditionalOptions': [
                                '/LTCG:INCREMENTAL', # incremental link-time code generation
                                ]
                            },
                            'VCLinkerTool': {
                                'OptimizeReferences': 2, # /OPT:REF
                                'EnableCOMDATFolding': 2, # /OPT:ICF
                                'LinkIncremental': 1, # disable incremental linking
                                'AdditionalOptions': [
                                '/LTCG:INCREMENTAL', # incremental link-time code generation
                                ],
                                'AdditionalLibraryDirectories': ['<(nwrfcsdk_dir)/lib'],
                                'AdditionalDependencies': ['sapnwrfc.lib', 'libsapucum.lib'],
                            },
                        },
                        'link_settings': {
                            'library_dirs': [
                                '<(nwrfcsdk_dir)/lib'
                            ],
                            'libraries': [
                                '-lsapnwrfc.lib',
                                '-lsapucum.lib',
                            ],
                        },
                        # 'libraries': [ '-lsapnwrfc.lib', '-llibsapucum.lib' ],
                    },
                ],
            ],
        }
    ],
}
