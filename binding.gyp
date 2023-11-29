# https://github.com/nodejs/node-addon-api/blob/main/doc/setup.md
# https://github.com/nodejs/node-gyp/blob/main/addon.gypi
# https://github.com/nodejs/node/blob/eaaee92d9b0be82d7f40b2abb67f30ce525d4bc4/common.gypi
# https://github.com/chromium/gyp/blob/md-pages/docs/InputFormatReference.md
# Darwin: https://developer.apple.com/library/archive/documentation/DeveloperTools/Reference/XcodeBuildSettingRef/1-Build_Setting_Reference/build_setting_ref.html#//apple_ref/doc/uid/TP40003931-CH3-SW105

{
    'variables': {
        # SAPNWRFC_HOME_CLOUD is used in cloud deployents, like on Cloud Foundry
        # https://blogs.sap.com/2023/10/26/abap-rfc-connectivity-from-btp-node.js-buildpack/
        'nwrfcsdk_dir': '<!(node -p "process.env.SAPNWRFC_HOME || process.env.SAPNWRFC_HOME_CLOUD")',
        'nwrfcsdk_include_dir': '<(nwrfcsdk_dir)/include',
        'nwrfcsdk_lib_dir': '<(nwrfcsdk_dir)/lib',
        'napi_include_dir': "<!(node -p \"require('node-addon-api').include_dir\")",
        'napi_version': "<!(node -p \"require('./package.json').config.napi_version\")",
        'node_abi_version': '<!(node -p "process.versions.modules")',
        # per NodeJS build requirements: https://github.com/nodejs/node/blob/main/BUILDING.md
        'macosx_version_min': '10.15',
        'cpp_standard': 'c++17',
        'target_name': 'sapnwrfc',
        'ccflags_defaults': [
            '-fno-rtti',
            '-Wno-unused-variable',
        ]
    },

    'target_defaults': {
        'type': 'loadable_module',
        'win_delay_load_hook': 'true',
        'product_prefix': '',
        'default_configuration': 'Release',
        'include_dirs': ['<(napi_include_dir)', '<(nwrfcsdk_include_dir)'],

        # C++ exceptions are enabled: https://github.com/nodejs/node-addon-api/blob/main/doc/setup.md
        'cflags!': [ '-fno-exceptions' ],
        'cflags_cc!': [ '-fno-exceptions'],

        'configurations': {
            'Debug': {
                'defines!': [
                    'NDEBUG'
                ],
                'cflags_cc!': [
                    '-O2',
                    '-Os',
                    '-DNDEBUG'
                ],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS!': [
                        '-O2',
                        '-Os',
                        '-DDEBUG'
                    ],
                    'GCC_OPTIMIZATION_LEVEL': '-1',
                    'GCC_GENERATE_DEBUGGING_SYMBOLS': 'YES'
                }
            },

            'Release': {
                'defines': [
                    'NDEBUG'
                ],

                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS!': [
                        '-Os',
                        '-O1'
                    ],
                    'GCC_OPTIMIZATION_LEVEL': '2',
                    'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
                    'DEAD_CODE_STRIPPING': 'YES',
                    'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES'
                }
            }
        }
    },

    'targets': [
        {
            'target_name': '<(target_name)',
            'sources': [
                'src/cpp/addon.cc',
                'src/cpp/Log.cc',
                'src/cpp/nwrfcsdk.cc',
                'src/cpp/Client.cc',
                'src/cpp/Pool.cc',
                'src/cpp/Server.cc',
                'src/cpp/server_api.cc',
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
                'NAPI_VERSION=<(napi_version)',
                'sapnwrfc_EXPORTS'
            ],
            'conditions': [
                [
                    'OS=="mac"',
                    {
                        'variables': {
                            'ccflags_mac': [
                                '-Wpedantic', '-Wall', '-Wextra', '-Werror', '-Wuninitialized', '-Wunreachable-code',
                                '-stdlib=libc++',
                                '-std=<(cpp_standard)',
                                '-mmacosx-version-min=<(macosx_version_min)',
                                '-fvisibility=hidden',
                                '-fPIC',
                                '-MD', '-MT'
                            ]
                        },
                        'cflags_cc': [
                            '<@(ccflags_defaults)'
                            '<@(ccflags_mac)'
                        ],
                        'defines': [
                            '_DARWIN_USE_64_BIT_INODE=1',
                            'SAPonDARW',
                            '__NO_MATH_INLINES',
                        ],
                        'link_settings': {
                            'library_dirs': [
                                '<(nwrfcsdk_lib_dir)'
                            ],
                            'libraries': [
                                '-lsapnwrfc',
                                '-lsapucum',
                                '-Wl,-rpath,<(nwrfcsdk_lib_dir)'
                            ],
                        },
                        'xcode_settings': {
                            'CLANG_CXX_LANGUAGE_STANDARD': '<(cpp_standard)',
                            'CLANG_CXX_LIBRARY': 'libc++',
                            'DYLIB_INSTALL_NAME_BASE': '@rpath',
                            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                            'GCC_SYMBOLS_PRIVATE_EXTERN': 'YES', # -fvisibility=hidden
                            'GCC_WARN_PEDANTIC': 'YES',
                            'MACOSX_DEPLOYMENT_TARGET': '<(macosx_version_min)',
                            'OTHER_CFLAGS': [
                                '-std=<(cpp_standard)',
                                '-Wl,-bind_at_load'
                            ],
                            'OTHER_CPLUSPLUSFLAGS': [
                                '<@(ccflags_defaults)',
                                '<@(ccflags_mac)'
                            ],
                        },
                    },
                ],
                [
                    'OS=="linux"',
                    {
                        'cflags_cc': [
                            '-std=<(cpp_standard)',
                            '-std=<(cpp_standard)',
                            '-Wall', '-Wextra', '-Werror', '-Wuninitialized', '-Wunreachable-code',
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
                                '<(nwrfcsdk_lib_dir)'
                            ],
                            'libraries': [
                                '-lsapnwrfc',
                                '-lsapucum',
                                '-Wl,-rpath,<(nwrfcsdk_lib_dir)'
                            ],
                        },
                    },
                ],
                [
                    'OS=="win"',
                    {
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
                            "_HAS_EXCEPTIONS=1"
                        ],
                        'msvs_configuration_attributes': {
                            'OutputDirectory': '$(SolutionDir)$(ConfigurationName)',
                            'IntermediateDirectory': '$(OutDir)\\obj',
                        },
                        'msvs_settings': {
                            'VCCLCompilerTool': {
                                'ExceptionHandling': 1,
                                'WholeProgramOptimization': 'true', # /GL, whole program optimization, needed for LTCG
                                'AdditionalOptions': [ '/std:<(cpp_standard)', ],
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
                                'AdditionalLibraryDirectories': ['<(nwrfcsdk_lib_dir)'],
                                'AdditionalDependencies': ['sapnwrfc.lib', 'libsapucum.lib'],
                            },
                        },
                    },
                ],
            ],
        }
    ]
}
