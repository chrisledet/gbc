add_subdirectory(SDL2)
add_library(extern INTERFACE)
target_link_libraries(extern INTERFACE SDL2::SLD2)
