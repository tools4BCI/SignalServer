
# -----------------------------------------------------------------------



unix {
    LIBS += -lboost_thread \
        -lboost_system \
        -lboost_filesystem \
        -lboost_program_options\
        -lSDL \
        -lusb-1.0 \
        -lcomedi

    HARDWARE_PLATFORM = $$system(uname -m)
    contains( HARDWARE_PLATFORM, x86_64 )::{
        # 64-bit Linux
        LIBS += -Lextern/lib/ticpp/linux  -lticpp_64 \
                -Lextern/lib/tia/linux/amd64 -ltia \
                -Lextern/lib/tobicore/linux/amd64 -ltobicore \
                -Lextern/lib/tobiid/linux/amd64 -ltobiid \
                -Lextern/lib/libtid/linux/amd64 -ltid

        contains( DEFINES, USE_GDF_SAVER )::LIBS += extern/lib/libgdf/linux/libGDF_64.a

    }else::{

        # 32-bit Linux
        LIBS += -Lextern/lib/ticpp/linux  -lticpp \
                -Lextern/lib/tia/linux/x86 -ltia \
                -Lextern/lib/tobicore/linux/x86 -ltobicore \
                -Lextern/lib/tobiid/linux/x86 -ltobiid \
                -Lextern/lib/libtid/linux/x86 -ltid

        contains( DEFINES, USE_GDF_SAVER )::LIBS += extern/lib/libgdf/linux/libGDF_32.a
    }
}


win32:LIBS += extern\lib\sdl\win\SDL.lib \
    extern\lib\sdl\win\SDLmain.lib \
    extern\lib\ticpp\win\ticpp.lib \
    kernel32.lib \
    advapi32.lib
