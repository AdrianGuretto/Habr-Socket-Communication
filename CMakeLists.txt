cmake_minimum_required(VERSION 3.20)
project(HabrArticleChat
	VERSION 1.0
	LANGUAGES CXX
)
add_executable(server_app server.cpp common.h)
add_executable(client_app client.cpp common.h)

if (WIN32)
	target_link_libraries(server_app PUBLIC -lws2_32)
	target_link_libraries(client_app PUBLIC -lws2_32)
endif()

target_compile_features(server_app PRIVATE cxx_std_17)
target_compile_features(client_app PRIVATE cxx_std_17)

