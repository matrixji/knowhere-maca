message(STATUS "FindCUDAToolkit for maca")

# cudart
if (MACA_PATH)
    set(CUDAToolkit_INCLUDE_DIRS
        ${MACA_PATH}/tools/wcuda/include
        ${MACA_PATH}/include
        ${MACA_PATH}/include/mcc
        ${MACA_PATH}/include/mcr
        ${MACA_PATH}/include/mcblas
        ${MACA_PATH}/include/common
    )

    add_library(MACA::mcruntime SHARED IMPORTED)
    find_library(MACA_MCRUNTIME NAMES mcruntime PATHS ${MACA_PATH}/lib REQUIRED)
    set_property(TARGET MACA::mcruntime PROPERTY IMPORTED_LOCATION ${MACA_MCRUNTIME})
    target_include_directories(MACA::mcruntime INTERFACE ${CUDAToolkit_INCLUDE_DIRS})

    add_library(MACA::wcuda_runtime SHARED IMPORTED)
    find_library(MACA_WCUDA_RUNTIME NAMES wcuda_runtime PATHS ${MACA_PATH}/lib)
    if (NOT MACA_WCUDA_RUNTIME)
        message(STATUS "Old maca without wcuda_runtime")
    else ()
        set_property(TARGET MACA::wcuda_runtime PROPERTY IMPORTED_LOCATION ${MACA_WCUDA_RUNTIME})
        target_include_directories(MACA::wcuda_runtime INTERFACE ${CUDAToolkit_INCLUDE_DIRS})
    endif ()

    add_library(CUDA::cudart INTERFACE IMPORTED)
    if (MACA_WCUDA_RUNTIME)
        set_property(TARGET CUDA::cudart PROPERTY INTERFACE_LINK_LIBRARIES MACA::mcruntime MACA::wcuda_runtime)
        message(STATUS "Found wrapper for CUDA::cudart ${MACA_MCRUNTIME} ${MACA_WCUDA_RUNTIME}")
    else ()
        set_property(TARGET CUDA::cudart PROPERTY INTERFACE_LINK_LIBRARIES MACA::mcruntime)
        message(STATUS "Found wrapper for CUDA::cudart ${MACA_MCRUNTIME}")
    endif ()

    add_library(CUDA::cublas SHARED IMPORTED)
    find_library(MACA_MCBLAS NAMES mcblas PATHS ${MACA_PATH}/lib REQUIRED)
    set_property(TARGET CUDA::cublas PROPERTY IMPORTED_LOCATION ${MACA_MCBLAS})
    target_include_directories(CUDA::cublas INTERFACE ${CUDAToolkit_INCLUDE_DIRS})
    message(STATUS "Found wrapper for CUDA::cublas ${MACA_MCBLAS}") 
endif (MACA_PATH)