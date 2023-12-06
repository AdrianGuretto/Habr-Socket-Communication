cmake_minimum_required(VERSION 3.20)
project(HabrArticleChat
	VERSION 1.0
	LANGUAGES CXX
)
add_executable(server_app server.cpp common.h)
add_executable(client_app client.cpp common.h)

target_compile_features(server_app PRIVATE cxx_std_17)
target_compile_features(client_app PRIVATE cxx_std_17)

