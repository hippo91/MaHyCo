set( LIBRARY_NAME "eucclhydremap" )

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR
      CMAKE_CXX_COMPILER_ID STREQUAL AppleClang)
    string(APPEND CMAKE_CXX_FLAGS " -Wno-c++11-narrowing")
endif()

add_library( ${LIBRARY_NAME} )
target_sources( ${LIBRARY_NAME} 
    PRIVATE
        EucclhydRemap.cc
        ConditionsLimites.cc
        Init.cc
        PhaseLagrange.cc
        PhaseRemapFinal.cc
        Utiles.cc
)
target_include_directories( ${LIBRARY_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR} )
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    target_link_libraries( ${LIBRARY_NAME} 
        PUBLIC
            cppnabla
            stdc++fs
    )
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    target_link_libraries( ${LIBRARY_NAME} 
        PUBLIC
            cppnabla
    )
    else()
    message( FATAL_ERROR "Not configured to be built on Windows" )
endif()
