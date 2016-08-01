
########### next target ###############

IF(KVSNS_PREFIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${KVSNS_PREFIX}/include")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -L${KVSNS_PREFIX}/lib")
ENDIF()

SET(fsalkvsns_LIB_SRCS
   fsal_internal.c
   main.c
   export.c
   handle.c
   file.c
   xattrs.c
)

add_library(fsalkvsns SHARED ${fsalkvsns_LIB_SRCS})

target_link_libraries(fsalkvsns
  kvsns hiredis
  ${SYSTEM_LIBRARIES}
)

set_target_properties(fsalkvsns PROPERTIES VERSION 4.2.0 SOVERSION 4)

########### install files ###############

install(TARGETS fsalkvsns COMPONENT fsal DESTINATION  ${FSAL_DESTINATION} )
