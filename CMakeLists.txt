project(HW2)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS -pthread)
add_executable(Server Server.cpp)
add_executable(Retriever Retriever.cpp)

add_executable(HW2 Server.cpp Retriever.cpp)