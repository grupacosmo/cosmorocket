idf_component_register(SRCS "minmea/minmea.c"
                       INCLUDE_DIRS ".")

# Fix unused function using timegm, which isn't available on ESP
target_compile_options(${COMPONENT_LIB} PRIVATE
    "-Dtimegm(x)=0"
)
