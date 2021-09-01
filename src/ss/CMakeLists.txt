file(GLOB SRC_FILES "./*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})

target_sources(${PROJECT_NAME_TEST} PRIVATE ${SRC_FILES})
