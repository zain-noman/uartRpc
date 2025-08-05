add_executable(cobsEncodeTest test_encode.c ../cobs.c)
target_include_directories(cobsEncodeTest PUBLIC ..)
add_test(NAME cobsEncodeTest COMMAND cobsEncodeTest)

add_executable(cobsDecodeTest test_decode.c ../cobs.c)
target_include_directories(cobsDecodeTest PUBLIC ..)
add_test(NAME cobsDecodeTest COMMAND cobsDecodeTest)