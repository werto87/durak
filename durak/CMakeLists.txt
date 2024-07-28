##
## Copyright (c) 2024 Waldemar Schneider (w-schneider1987 at web dot de)
##
## Distributed under the Boost Software License, Version 1.0.
## (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


add_library(durak INTERFACE)


add_library(myproject::durak ALIAS durak)


find_package(confu_json)
find_package(Boost)


target_link_libraries(durak INTERFACE
        confu_json::confu_json
        boost::boost
        )

target_include_directories(durak INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>)

target_compile_features(durak INTERFACE cxx_std_20)

install(FILES
        card.hxx
        constant.hxx
        game.hxx
        gameData.hxx
        gameOption.hxx
        move.hxx
        player.hxx
        print.hxx
        DESTINATION include/durak
        )
install(TARGETS durak DESTINATION lib)
