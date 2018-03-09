# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
PROJECT(HelloWorld)

include_directories("src/main/jni/include")
link_directories("src/main/jniLibs/armeabi")

add_library(ffmpeg
            SHARED
            src/main/jni/FFmpegUtils.cpp
            src/main/jni/IRecorder.cpp
            src/main/jni/VideoRecorder.cpp
            src/main/jni/AudioRecorder.cpp
            src/main/jni/NativeByteBuffer.cpp
            src/main/jni/NativeByteBufferWrapper.cpp
            src/main/jni/BuffersStorage.cpp
            src/main/jni/BuffersStorageWrapper.cpp
            src/main/jni/FFmpegRecorder.cpp
            src/main/jni/FFmpegRecorderWrapper.cpp
            src/main/jni/jni.c
            )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(
ffmpeg
${log-lib}
avcodec
avfilter
avformat
avutil
swresample
swscale
)
