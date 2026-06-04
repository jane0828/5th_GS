find_path(Freetype2_INCLUDE_DIRS freetype/freetype.h
    # 검색할 패스
    PATHS
        # 환경 변수 Freetype_ROOT 또는 Freetype_INCLUDE_DIR가 존해한다면 그기를 검색
        ENV Freetype2_ROOT
        ENV Freetype2_INCLUDE_DIR
        # CMake의 변수로서 Freetype_ROOT를 정의 되어 있다면 그기를 검색
        ${Freetype2_ROOT}
        /usr/freetype2
        /usr/local/freetype2
        /usr/include/freetype2
    PATH_SUFFIXES
        include
)
# # 라이브러리 패스를 라이브러리 이름을 토대로 검색한다
# find_library(Freetype2_LIBRARY
#     NAMES
#         Freetype2
#     PATHS
#         ENV Freetype2_ROOT
#         ENV Freetype2_LIB_DIR
#         ${Freetype2_ROOT}
#         /usr
#         /usr/local
#         /usr/include
#     PATH_SUFFIXES
#         lib
# )
# advanced 모드가 아닌한 변수의 존재를 찾는다
mark_as_advanced(Freetype2_INCLUDE_DIRS Freetype2_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Freetype2
    REQUIRED_VARS
        Freetype2_INCLUDE_DIRS
)

# Freetype를 발견하지만 Freetype::Freetype가 정의 되어 있지 않은 경우
if(Freetype2_FOUND AND NOT TARGET Freetype2::Freetype2)
    # Freetype::Freetype라는 타겟 이름으로 Freetype 라이브러리를 정의
    # UNKNOWN = STATIC/SHARED 인지는 아직 불명
    # IMPORTED = 이 프로젝트를 속하지 않은 타겟
    add_library(Freetype2::Freetype2 UNKNOWN IMPORTED)
    set_target_properties(Freetype2::Freetype2 PROPERTIES
    # C언어, C++ 이라면 "CXX"라고 한다
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    IMPORTED_LOCATION "${Freetype2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${Freetype2_INCLUDE_DIR}"
)
endif()