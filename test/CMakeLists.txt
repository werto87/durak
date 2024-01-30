add_executable(_test
        constant.hxx
        game.cxx
        main.cxx
        move.cxx
        player.cxx
        )
find_package(Catch2)
target_link_libraries(_test
        myproject_options
        myproject_warnings
        durak
        Catch2::Catch2WithMain
        )
target_include_directories(_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})

add_test(NAME _test
        COMMAND _test -r junit)