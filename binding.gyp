{
  "targets": [
    {
      "target_name": "deadlinemath",
      "sources": [
        "src/wrapper.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")",
        'burstmath'
      ],
      'cflags': ['-fPIC', '-std=c99', '-Wall', '-m64', '-O3', '-mtune=native'],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_CPP_EXCEPTIONS"]
    }, {
      "target_name": "deadlinemath_sse4",
      "sources": [
        "src/wrapper_sse4.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")",
        'burstmath_sse4'
      ],
      'cflags': ['-fPIC', '-std=c99', '-Wall', '-m64', '-O3', '-mtune=native'],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_CPP_EXCEPTIONS"]
    }, {
      "target_name": "deadlinemath_avx2",
      "sources": [
        "src/wrapper_avx2.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")",
        'burstmath_avx2'
      ],
      'cflags': ['-fPIC', '-std=c99', '-Wall', '-m64', '-O3', '-mtune=native'],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "defines": ["NAPI_CPP_EXCEPTIONS"]
    }, {
        'target_name': 'burstmath_avx2',
        'type': 'static_library',
        'sources': [
          'src/mshabal256_avx2.c',
          'src/mshabal_sse4.c',
          'src/sph_shabal.c',
          'src/burstmath_avx2.c',
        ],
        'cflags': ['-fPIC', '-mavx2', '-std=c99', '-Wall', '-m64', '-O3', '-mtune=native'],
        "include_dirs": [
          "<!@(node -p \"require('node-addon-api').include\")"
        ],
        "dependencies": [
          "<!(node -p \"require('node-addon-api').gyp\")"
        ],
    }, {
         'target_name': 'burstmath_sse4',
         'type': 'static_library',
         'sources': [
           'src/mshabal_sse4.c',
           'src/sph_shabal.c',
           'src/burstmath_sse4.c',
         ],
         'cflags': ['-fPIC', '-std=c99', '-Wall', '-m64', '-O3', '-mtune=native'],
         "include_dirs": [
           "<!@(node -p \"require('node-addon-api').include\")"
         ],
         "dependencies": [
           "<!(node -p \"require('node-addon-api').gyp\")"
         ],
     }, {
           'target_name': 'burstmath',
           'type': 'static_library',
           'sources': [
             'src/sph_shabal.c',
             'src/burstmath.c',
           ],
           'cflags': ['-fPIC', '-std=c99', '-Wall', '-m64', '-O3', '-mtune=native'],
           "include_dirs": [
             "<!@(node -p \"require('node-addon-api').include\")"
           ],
           "dependencies": [
             "<!(node -p \"require('node-addon-api').gyp\")"
           ],
       }
  ]
}