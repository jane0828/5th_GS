find_path(SSH_INCLUDE_DIRS libssh.h
    # 검색할 패스
    PATHS
        # 환경 변수 SSH_ROOT 또는 SSH_INCLUDE_DIR가 존해한다면 그기를 검색
        ENV SSH_ROOT
        ENV SSH_INCLUDE_DIR
        # CMake의 변수로서 SSH_ROOT를 정의 되어 있다면 그기를 검색
        ${SSH_ROOT}
        /usr/libssh
        /usr/local/libssh
        /usr/include/libssh
    PATH_SUFFIXES
        include
)
# 라이브러리 패스를 라이브러리 이름을 토대로 검색한다
find_library(SSH_LIBRARY
    NAMES
        ssh
    PATHS
        ENV SSH_ROOT
        ENV SSH_LIB_DIR
        ${SSH_ROOT}
        /usr/libssh
        /usr/local/libssh
        /usr/include/libssh
    PATH_SUFFIXES
        lib
)
# advanced 모드가 아닌한 변수의 존재를 찾는다
mark_as_advanced(SSH_INCLUDE_DIRS SSH_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SSH
    REQUIRED_VARS
        SSH_INCLUDE_DIRS
        SSH_LIBRARY
)

# SSH를 발견하지만 SSH::SSH가 정의 되어 있지 않은 경우
if(SSH_FOUND AND NOT TARGET SSH::SSH)
    # SSH::SSH라는 타겟 이름으로 SSH 라이브러리를 정의
    # UNKNOWN = STATIC/SHARED 인지는 아직 불명
    # IMPORTED = 이 프로젝트를 속하지 않은 타겟
    add_library(SSH::SSH UNKNOWN IMPORTED)
    set_target_properties(SSH::SSH PROPERTIES
    # C언어, C++ 이라면 "CXX"라고 한다
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    IMPORTED_LOCATION "${SSH_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SSH_INCLUDE_DIR}"
)
endif()