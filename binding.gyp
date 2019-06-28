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
        'target_name': 'burstmath',
        'type': 'static_library',
        'sources': [
          'src/mshabal_sse4.c',
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