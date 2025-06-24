# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.22)

# CMAKE默认设置release版
set(CMAKE_BUILD_TYPE"Release" CACHE STRING"" FORCE)

project ("start")

set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CATKIN_TOPLEVEL TRUE)
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD 20)

# 开启C++20 export/module
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")

# 内存泄露检测
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts -fsanitize=address -fno-omit-frame-pointer")
# 内存检测
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# 输出项目信息
message(STATUS ${PROJECT_NAME} ${PROJECT_VERSION_MAJOR} ${PROJECT_VERSION_MINOR})

# 三方库开始
# 头文件
# python三方库相关
set(Python_INCLUDE_DIRS ${Python_DIR}/include)  
set(Python_LIB_DIRS ${Python_DIR}/lib)  

set(VCPKG_LIB ${VCPKG_ROOT}/installed/x64-linux/lib)
set(VCPKG_INCLUDE ${VCPKG_ROOT}/installed/x64-linux/include)
set(LOCAL_INCLUDE /usr/local/include)
set(GLOBAL_INCLUDE /usr/include)

include_directories(VCPKG_INCLUDE)
include_directories(LOCAL_INCLUDE)
include_directories(GLOBAL_INCLUDE)

# SQLite Modern C++
# set(SQLITE_MODERN_INCLUDE_PATH ${VCPKG_ROOT}/packages/sqlite-modern-cpp_x64-linux)
# include_directories(${SQLITE_MODERN_INCLUDE_PATH}/include)

# openssl封装库
find_package(cryptopp CONFIG REQUIRED)
# 对zip解压的支持
find_package(nlohmann_json REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(fmt REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(PythonInterp REQUIRED)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
find_package(Crow)
find_package(Boost REQUIRED COMPONENTS coroutine)
find_path(SQLITE_MODERN_CPP_INCLUDE_DIRS "sqlite_modern_cpp.h")
include_directories(${SQLITE_MODERN_CPP_INCLUDE_DIRS})
# find_package(libgo CONFIG REQUIRED)
#POCO引入
find_package(Poco REQUIRED COMPONENTS Foundation Zip Util)

add_definitions(-DCROW_ENABLE_SSL)
set(CROW_ENABLE_SSL ON)

# INCLUDE三方库
set(JWT_PATH     ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/jwt-cpp-master)
include_directories(${JWT_PATH}/include)

set(AES_PATH     ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/AES)
include_directories(${AES_PATH}/include)

set(BCRYPT_PATH     ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Bcrypt-cpp-master)
include_directories(${BCRYPT_PATH}/include)

include_directories(${Poco_INCLUDE_DIRS}})

# 导入a
link_directories(${BCRYPT_PATH}/lib)
link_directories(${GOLIB_PATH}/lib)
link_directories(${VCPKG_LIB})
# 三方库结束

# 指定最终的可执行文件的位置
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_ROOT_NODE}/bin)
# 设置第三方库编译文件输出路径
set(LIBRARY_OUTPUT_DIRECTORY ${OPENCV_LIB_SHARED_PATH}/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
# 将源代码添加到此项目的可执行文件。
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)
add_executable (${PROJECT_NAME} ${SRC_LIST})

#add_subdirectory(openssl)  
#Poco::Net Poco::JSON Poco::Util
#target_include_directories(${PROJECT_NAME} private ${QRENCODE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC ${OpenCV_LIBS} ${OPENSSL_LIBRARIES} ${QRENCODE_LIBRARY_RELEASE} ${Poco_LIBRARIES}
${SQLITE_MODERN_CPP_INCLUDE_DIRS}
Crow::Crow cpprestsdk::cpprest nlohmann_json::nlohmann_json
fmt::fmt sqlite3 unofficial::sqlite3::sqlite3 yaml-cpp Boost::coroutine
spdlog::spdlog cryptopp::cryptopp #libgo::libgo
libbcrypt.a)

message(G:${CMAKE_C_COMPILER_VERSION},GXX:${CMAKE_CXX_COMPILER_VERSION},CXX STANDARD:${CMAKE_CXX_STANDARD})
message(final)