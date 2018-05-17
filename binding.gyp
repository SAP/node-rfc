{
    "variables": {
        "sapnwrfcsdk_path_linux": "$(SAPNWRFC_HOME)",
        "sapnwrfcsdk_path_windows": "<!(echo %SAPNWRFC_HOME%)",
        "build_dir": '<(PRODUCT_DIR)',
        "conditions": [
            ['OS=="win"', {"build_dir": "<(PRODUCT_DIR)/../win32_x64"}]
        ]
    },
    "targets": [
        {
            "target_name": "<(module_name)",

            "sources": [
                "src/node_sapnwrfc.cc",
                "src/client.cc",
                "src/rfcio.cc",
                "src/error.cc"
            ],

            "cflags!": ["-fno-exceptions"],
            "cflags_cc!": ["-fno-exceptions"],

            "defines": [
                "SAPwithUNICODE",
                "SAPwithTHREADS",
                "NDEBUG",
                'NAPI_CPP_EXCEPTIONS'
            ],

            "dependencies": ["<!(node -p \"require('node-addon-api').gyp\")"],
            "include_dirs": ["<!@(node -p \"require('node-addon-api').include\")"],

            "conditions": [

                ['OS=="linux"', {
                    "cflags_cc!": [
                        "-Wall"
                    ],
                    "defines": [
                        "SAPwithUNICODE",
                        "SAPwithTHREADS",
                        "SAPonLIN",
                        "SAPonUNIX",
                        "_LARGEFILE_SOURCE",
                        "_FILE_OFFSET_BITS=64",
                        "__NO_MATH_INLINES"
                    ],
                    "ldflags": [
                        "-Wl,-rpath,'$$ORIGIN'"
                    ],
                    "link_settings": {
                        "libraries": ["-L<(sapnwrfcsdk_path_linux)/lib", "-lsapnwrfc", "-lsapucum"]
                    },
                    "include_dirs": [
                        "<(sapnwrfcsdk_path_linux)/include/"
                    ]
                }
                ],


                ['OS=="win"', {
                    'defines': [
                        'PLATFORM="win32"',
                        'WIN32',
                        '_AFXDLL',
                        '_CRT_NON_CONFORMING_SWPRINTFS',
                        '_CRT_SECURE_NO_DEPRECATE',
                        '_CRT_NONSTDC_NO_DEPRECATE',
                        'SAPonNT',
                        'UNICODE',
                        '_UNICODE'
                    ],
                    'conditions': [
                        ['target_arch=="ia32"', {'defines': [
                            '_X86_'], 'product_dir': 'win32_x86'}],
                        ['target_arch=="x64"',  {'defines':  [
                            '_AMD64_'], 'product_dir': 'win32_x64'}]
                    ],
                    'include_dirs': [
                        '<(sapnwrfcsdk_path_windows)/include',
                        "<!(node -e \"require('node-addon-api')\")"
                    ],
                    'msvs_configuration_attributes': {
                        'OutputDirectory': '$(SolutionDir)$(ConfigurationName)',
                        'IntermediateDirectory': '$(OutDir)\\obj'
                    },
                    'msvs_settings': {
                        'VCLinkerTool': {
                            'AdditionalLibraryDirectories': ['<(sapnwrfcsdk_path_windows)/lib'],
                            'AdditionalDependencies': ['sapnwrfc.lib', 'libsapucum.lib']
                        },
                    },
                    # 'libraries': [ '-lsapnwrfc.lib', '-llibsapucum.lib' ],
                }]

            ]
        },
        {
            "target_name": "action_after_build",
            "type": "none",
            "dependencies": ["<(module_name)"],
            "copies": [
                {
                    "files": ["<(build_dir)/<(module_name).node"],
                    "destination": "<(module_path)"
                }
            ]
        }
    ]
}
