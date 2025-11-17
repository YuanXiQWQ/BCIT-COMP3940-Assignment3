cmake_minimum_required(VERSION 3.16)
project(UploadServerCpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(upload_server
    UploadServer.cpp
    ServerSocket.cpp
    Socket.cpp
    Thread.cpp
    HttpServlet.cpp
    HttpServletRequest.cpp
    HttpServletResponse.cpp
    UploadServlet.cpp
    UploadServerThread.cpp
    MultipartParser.cpp
)

if (WIN32)
    target_link_libraries(upload_server ws2_32)
endif()
