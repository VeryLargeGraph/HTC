cmake_minimum_required(VERSION 3.16)
project(Hypergraph_TriangleCounting)

find_package(OpenMP REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -O3")

#add_definitions(-DDebug)

include_directories(./include)

add_executable(GraphInfo src/GraphInfo.cpp)

add_executable(Motifs src/Motifs.cpp)
add_executable(Motifs_Parallel src/Motifs.parallel.cpp)
target_compile_options(Motifs_Parallel PRIVATE "${OpenMP_CXX_FLAGS}")
target_link_libraries(Motifs_Parallel PRIVATE "${OpenMP_CXX_FLAGS}")

add_executable(Baseline src/Baseline.cpp)
target_compile_options(Baseline PRIVATE "${OpenMP_CXX_FLAGS}")
target_link_libraries(Baseline PRIVATE "${OpenMP_CXX_FLAGS}")

add_executable(PushForward src/PushForward.cpp)
target_compile_options(PushForward PRIVATE "${OpenMP_CXX_FLAGS}")
target_link_libraries(PushForward PRIVATE "${OpenMP_CXX_FLAGS}")

add_executable(PrefixForest src/PrefixForest.cpp)
target_compile_options(PrefixForest PRIVATE "${OpenMP_CXX_FLAGS}")
target_link_libraries(PrefixForest PRIVATE "${OpenMP_CXX_FLAGS}")

# add_executable(Test src/Test.cpp)
# target_compile_options(Test PRIVATE "${OpenMP_CXX_FLAGS}")
# target_link_libraries(Test PRIVATE "${OpenMP_CXX_FLAGS}")

# add_executable(Test2 src/Test2.cpp)
# target_compile_options(Test2 PRIVATE "${OpenMP_CXX_FLAGS}")
# target_link_libraries(Test2 PRIVATE "${OpenMP_CXX_FLAGS}")