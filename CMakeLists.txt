idf_component_register(
    SRCS 
        "source/epd_basic.c"
        "source/epd_paint.cpp"
        "source/epd_spi.c"
    
        "fonts/font8.cpp"
        "fonts/font12.cpp"
        "fonts/font16.cpp"
        "fonts/font20.cpp"
        "fonts/font24.cpp"
        # "fonts/font12CN.c"
        # "fonts/font24CN.c"

    INCLUDE_DIRS 
        "."
        "include"
        "fonts"
)