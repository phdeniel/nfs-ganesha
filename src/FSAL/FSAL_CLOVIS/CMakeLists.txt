
########### next target ###############
add_subdirectory(m0store)

IF(ZFS_PREFIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I${ZFS_PREFIX}/include")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -L${ZFS_PREFIX}/lib")
ENDIF()

SET(fsalclovis_LIB_SRCS
   fsal_internal.c
   main.c
   export.c
   handle.c
   file.c
   xattrs.c
   external_storage.c
)

add_library(fsalclovis SHARED ${fsalclovis_LIB_SRCS})

target_link_libraries(fsalclovis
  zfswrap
  m0store
  ${SYSTEM_LIBRARIES}
)

set_target_properties(fsalclovis PROPERTIES VERSION 4.2.0 SOVERSION 4)

########### install files ###############

install(TARGETS fsalclovis COMPONENT fsal DESTINATION  ${FSAL_DESTINATION} )
