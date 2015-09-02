{
  "variables": {
    "linux_nwrfcsdk_path": "/usr/local/sap/nwrfcsdk",
    "msvs_nwrfcsdk_path": "c:\\nwrfcsdk-64",
    # _todo: http://stackoverflow.com/questions/17023442/referring-to-environment-variables-from-binding-gyp-node-gyp
    # "msvs_nwrfcsdk_path": '! (echo %SAPNWRFC_HOME%)'
  },

  "targets": [
    {
      "target_name": "rfc",

      "sources": [ "src/rfc.cc", "src/error.cc", "src/rfcio.cc", "src/Client.cc" ],
      "defines": [
        "SAPwithUNICODE",
        "SAPwithTHREADS",
        "NDEBUG"
      ],

      "conditions": [

            [ 'OS=="linux"', {
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
                "link_settings": {
                    "libraries": ["-L<(linux_nwrfcsdk_path)/lib", "-lsapnwrfc", "-lsapucum"]
                },
                "include_dirs": [
                    "<(linux_nwrfcsdk_path)/include/"
                ],
                'product_dir' : 'linux_x64'
            }],


            [ 'OS=="win"', {
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
                    [ 'target_arch=="ia32"' , {'defines' : ['_X86_']  , 'product_dir': 'win32_x86' }],
                    [ 'target_arch=="x64"' ,  {'defines':  ['_AMD64_'], 'product_dir': 'win32_x64' }]
                ],
                'include_dirs': [
                    '<(msvs_nwrfcsdk_path)/include'
                ],
                'msvs_configuration_attributes': {
                    'OutputDirectory': '$(SolutionDir)$(ConfigurationName)',
                    'IntermediateDirectory': '$(OutDir)\\obj'
                },
                'msvs_settings': {
                    'VCLinkerTool': {
                    'AdditionalLibraryDirectories': [ '<(msvs_nwrfcsdk_path)/lib' ],
                    'AdditionalDependencies': [ 'sapnwrfc.lib', 'libsapucum.lib' ]
                    },
                },
                #'libraries': [ '-lsapnwrfc.lib', '-llibsapucum.lib' ],
            }]

      ] # conditions

    }
  ]
}
