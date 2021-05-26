if (MSVC)
    find_path(MAILIO_INCLUDE_DIR mailio PATHS ENV VCPKG_ROOT)
    find_file(MAILIO_LIB lib/mailio.lib PATHS ENV VCPKG_ROOT)
else ()
    set(MAILIO_INCLUDE_DIR /mnt/c/dev/vcpkg/installed/x64-linux/include/) # TODO: Make it auto config
    set(MAILIO_LIB /mnt/c/dev/vcpkg/installed/x64-linux/lib/libmailio.a)
endif ()
