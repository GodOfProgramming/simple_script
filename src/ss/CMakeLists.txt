set(SHARED_SOURCES
  cfg.cpp
  lib.cpp
  code.cpp
  datatypes.cpp
  exceptions.cpp
)

target_sources(${EXE} PUBLIC ${SHARED_SOURCES})

target_sources(${EXE_TEST} PUBLIC ${SHARED_SOURCES})
