{
    'variables': {
        'nwrfcsdk_dir': '$(SAPNWRFC_HOME)',
        'napi_include_dir': '<!(node -p \'require("node-addon-api").include_dir\')',
        'target_name': 'sapnwrfc',
        'conditions': [
            ['OS=="win"',
                {
                    'build_dir': '<(PRODUCT_DIR)/../win32_x64',
                    'nwrfcsdk_dir': '<!(echo %SAPNWRFC_HOME%)'
                }
            ]
        ],
    },

    'target_defaults': {
        'default_configuration': 'Release',

        'cflags_cc' : [
          '-std=c++17'
        ],

        'cflags_cc!': ['-std=gnu++0x','-fno-rtti', '-fno-exceptions'],

        'configurations': {
            'Debug': {
                'defines!': [
                    'NDEBUG'
                ],
                'cflags_cc!': [
                    '-O3',
                    '-Os',
                    '-DNDEBUG'
                ],
                'xcode_settings': {
                    'OTHER_CPLUSPLUSFLAGS!': [
                        '-O3',
                        '-Os',
                        '-DDEBUG'
                    ],
                    'GCC_OPTIMIZATION_LEVEL': '0',
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
                        '-O2'
                    ],
                    'GCC_OPTIMIZATION_LEVEL': '3',
                    'GCC_GENERATE_DEBUGGING_SYMBOLS': 'NO',
                    'DEAD_CODE_STRIPPING': 'YES',
                    'GCC_INLINES_ARE_PRIVATE_EXTERN': 'YES'
                }
            }
        }
    }
}