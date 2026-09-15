cmake_minimum_required(VERSION 3.8)

# set version number
set (OPENTYRIAN_VERSION 2.1)

# set the project name
project(OpenTyrian VERSION ${OPENTYRIAN_VERSION})

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Options enabled by default
option ( enable-network "compile support for network" on )

# Include required scripts
include(GNUInstallDirs)

# Search for dependencies
find_package(SDL2 REQUIRED)

if (enable-network)
    find_package(SDL2_net REQUIRED)
    add_definitions(-DWITH_NETWORK)
endif()

# add the executable
add_executable(opentyrian WIN32
    src/animlib.c
    src/arg_parse.c
    src/backgrnd.c
    src/config.c
    src/config_file.c
    src/destruct.c
    src/editship.c
    src/episodes.c
    src/file.c
    src/font.c
    src/fonthand.c
    src/game_menu.c
    src/helptext.c
    src/joystick.c
    src/jukebox.c
    src/keyboard.c
    src/lds_play.c
    src/loudness.c
    src/lvllib.c
    src/lvlmast.c
    src/mainint.c
    src/menus.c
    src/mouse.c
    src/mtrand.c
    src/musmast.c
    src/network.c
    src/nortsong.c
    src/nortvars.c
    src/opentyr.c
    src/opl.c
    src/palette.c
    src/params.c
    src/pcxload.c
    src/pcxmast.c
    src/picload.c
    src/player.c
    src/shots.c
    src/sizebuf.c
    src/sndmast.c
    src/sprite.c
    src/starlib.c
    src/tyrian2.c
    src/varz.c
    src/vga256d.c
    src/vga_palette.c
    src/video.c
    src/video_scale.c
    src/video_scale_hqNx.c
    src/xmas.c
    # Windows-only
    visualc/resources.rc
)

# Add extra definitions for Windows targets
if (WIN32)
    target_compile_definitions(opentyrian PUBLIC
        _CRT_SECURE_NO_WARNINGS
        TARGET_WIN32
        TYRIAN_DIR=\"C:\\\\TYRIAN\"
    )
elseif (UNIX)
    target_compile_definitions(opentyrian PUBLIC
        TARGET_UNIX
        TYRIAN_DIR="${DATADIR}/games/tyrian"
    )
endif()

# Setup version
target_compile_definitions(opentyrian PUBLIC
    OPENTYRIAN_VERSION=\"${OPENTYRIAN_VERSION}\"
)

# Compile options
target_include_directories(opentyrian
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(opentyrian
    SDL2::SDL2
    SDL2::SDL2main
    $<$<AND:$<BOOL:${enable-network}>,$<BOOL:${SDL2_net_FOUND}>>:
        SDL2_net::SDL2_net
    >
)

find_library(MATH_LIBRARY m)

if (MATH_LIBRARY)
    message(STATUS "Add math library ${MATH_LIBRARY}")
    target_link_libraries(opentyrian
        ${MATH_LIBRARY}
    )
endif()

if (UNIX)
    # Install the manual
    install(FILES linux/man/opentyrian.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6/opentyrian.6)

    # Install .desktop file
    install(FILES linux/opentyrian.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

    # Install icons
    foreach(SIZE 22 24 32 48 128)
        install(FILES linux/icons/tyrian-${SIZE}.png DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/${SIZE}x${SIZE}/apps)
    endforeach()
endif()
