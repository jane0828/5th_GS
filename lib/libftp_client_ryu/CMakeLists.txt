set(FTP_RYU_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/port")
set(FTP_RYU_GSCSP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../gscsp")

# Do not inherit the product executable's global OpenGL/UI link dependencies.
set_property(DIRECTORY PROPERTY LINK_LIBRARIES "")

set(FTP_RYU_SOURCES
    "${FTP_RYU_ROOT}/client/src/ftpnew_client.c"
    "${FTP_RYU_ROOT}/client/src/ftpnew_client_crc32.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/adapter/src/ryu_ftp_client_adapter.c"
)

option(FTP_RYU_CONN_DIAG "Enable Ryu FTP CSP connection lifecycle diagnostics" OFF)

if(FTP_RYU_CONN_DIAG)
    list(APPEND FTP_RYU_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/src/ryu_ftp_connection_diag.c"
    )
endif()

foreach(source IN LISTS FTP_RYU_SOURCES)
    if(NOT EXISTS "${source}")
        message(FATAL_ERROR "ftp_client_ryu source is missing: ${source}")
    endif()
endforeach()

add_library(ftp_client_ryu STATIC ${FTP_RYU_SOURCES})
add_library(FTP_Ryu::client ALIAS ftp_client_ryu)

set_target_properties(ftp_client_ryu PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS YES
    OUTPUT_NAME ftp_client_ryu
)

target_include_directories(ftp_client_ryu
    PUBLIC
        "${FTP_RYU_ROOT}/client/inc"
        "${FTP_RYU_ROOT}/config"
        "${CMAKE_CURRENT_SOURCE_DIR}/adapter/include"
        "${FTP_RYU_GSCSP_ROOT}/lib/libftp_client/include"
        "${FTP_RYU_GSCSP_ROOT}/lib/libutil/include"
    PRIVATE
        "${FTP_RYU_GSCSP_ROOT}/build"
        "${FTP_RYU_GSCSP_ROOT}/lib/libgscsp/include"
        "${FTP_RYU_GSCSP_ROOT}/lib/libgscsp/lib/libcsp/include"
        "${FTP_RYU_GSCSP_ROOT}/lib/libgscsp/lib/libcsp/src"
        "${FTP_RYU_GSCSP_ROOT}/build/lib/libgscsp/lib/libcsp/include"
)

if(FTP_RYU_CONN_DIAG)
    target_compile_definitions(ftp_client_ryu PUBLIC FTP_RYU_CONN_DIAG=1)
endif()

set_source_files_properties(
    "${FTP_RYU_ROOT}/client/src/ftpnew_client.c"
    PROPERTIES COMPILE_OPTIONS
        "-include;${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/ryu_ftp_compat.h"
)

set(FTP_RYU_GSCSP_ARCHIVE "${FTP_RYU_GSCSP_ROOT}/build/libmimancsp.a")
if(NOT EXISTS "${FTP_RYU_GSCSP_ARCHIVE}")
    message(FATAL_ERROR "gscsp dependency archive is missing: ${FTP_RYU_GSCSP_ARCHIVE}")
endif()

add_library(ftp_client_ryu_gscsp STATIC IMPORTED GLOBAL)
set_target_properties(ftp_client_ryu_gscsp PROPERTIES
    IMPORTED_LOCATION "${FTP_RYU_GSCSP_ARCHIVE}"
)
target_link_libraries(ftp_client_ryu PUBLIC ftp_client_ryu_gscsp)
