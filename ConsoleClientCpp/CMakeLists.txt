cmake_minimum_required(VERSION 3.16)
project(UploadConsoleClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(upload_client
    main.cpp
    MultipartForm.cpp
    MultipartForm.h
    HttpClient.cpp
    HttpClient.h
    Url.cpp
    Url.h
)

if (WIN32)
    target_link_libraries(upload_client ws2_32)
endif()
