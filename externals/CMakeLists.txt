add_subdirectory(SDL)
add_library(extern INTERFACE)
target_link_libraries(extern INTERFACE SDL3::SLD3)
