idf_component_register(SRCS "marker_event_loop.c" "marker_mode.c" "mode_stack_manager.c" "main.c" "event_sources/sense_source.c" "fs.c" "config_manager.c"
                    INCLUDE_DIRS ".")



# create nvs image from nvs_data.csv
nvs_create_partition_image(nvs ../nvs_data.csv FLASH_IN_PROJECT)