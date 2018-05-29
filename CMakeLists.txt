cmake_minimum_required (VERSION 3.1)
#do not lower cmake due to Threads requirements!
project(venice)
set(CMAKE_CXX_STANDARD 11)

if(EXISTS "$ENV{WORKSPACE}/sysconfig/cmake/physycom_config.cmake")
   message(STATUS "Enabling physycom settings")
   include("$ENV{WORKSPACE}/sysconfig/cmake/physycom_config.cmake")
else()
   message(STATUS "Unable to find physycom settings file")
endif()

if(UNIX AND NOT APPLE)
  find_package(LibRt REQUIRED)
endif()
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(FLTK REQUIRED)
find_package(Boost REQUIRED)
find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/ ${OpenCV_INCLUDE_DIRS})
link_directories(${FLTK_LIBRARY_DIR})

add_executable(heatmapper           "${CMAKE_SOURCE_DIR}/src/GLtrans.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/Globalmaptiles.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/draw.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/form.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/frame.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/leggi_dati.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/main.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/mappa.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/metodi.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/nodo.cpp"
                                    "${CMAKE_SOURCE_DIR}/src/polyline.cpp"
                                    )
target_link_libraries(heatmapper    ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} ${FLTK_LIBRARIES} Threads::Threads ${OpenCV_LIBRARIES})
install(TARGETS heatmapper DESTINATION ${CMAKE_SOURCE_DIR})