find_package(Threads REQUIRED)
add_library(thrax2 alignment.cpp label.cpp phrasalrule.cpp phrase.cpp tree.cpp)
target_compile_features(thrax2 PUBLIC cxx_std_17)
target_compile_options(thrax2 PRIVATE -Wall -Wextra -Werror -pedantic)

add_executable(hiero hiero.cpp)
target_link_libraries(hiero PRIVATE thrax2 Threads::Threads)
add_executable(samt samt.cpp)
target_link_libraries(samt PRIVATE thrax2 Threads::Threads)
