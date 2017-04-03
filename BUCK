cxx_library(
  name = 'safercplusplus',
  licenses = [
    'LICENSE_1_0.txt',
  ],
  header_namespace = '',
  exported_headers = subdir_glob([
    ('', '*.h'),
  ]),
  srcs = glob([
    '*.cpp',
  ],
  excludes = [
    'msetl_example.cpp',
  ]),
  compiler_flags = [
    '-std=c++14',
  ],
  visibility = [
    'PUBLIC',
  ],
)

cxx_binary(
  name = 'msetl_example',
  srcs = [
    'msetl_example.cpp',
  ],
  compiler_flags = [
    '-std=c++14',
  ],
  deps = [
    ':safercplusplus',
  ],
)
