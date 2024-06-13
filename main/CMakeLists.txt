file(GLOB marker_SRC
     "*.c"
)

idf_component_register(SRCS ${marker_SRC}
                    INCLUDE_DIRS ".")



# create nvs image from nvs_data.csv
if (NOT IDF_TARGET STREQUAL "linux")
    nvs_create_partition_image(nvs ../nvs_data.csv FLASH_IN_PROJECT)
endif()