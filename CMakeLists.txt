set(name Supera)


# Get all the source files:
file(GLOB SOURCES *.cxx)
file(GLOB HEADERS *.h)



#find_package(Geant4 REQUIRED)
#find_package(EDepSim)
#find_package(PythonLibs REQUIRED)
#find_package(ROOT CONFIG REQUIRED)

# Add a shared library
add_library(${name} OBJECT ${SOURCES})

#include("${ROOT_USE_FILE}")

#target_link_libraries(Supera ${ROOT_LIBRARIES})

#MINE ?????????
#target_link_libraries(Supera PUBLIC ROOT::Physics)
#target_link_libraries( Supera PUBLIC EDepSim::edepsim_io )
#target_link_libraries( Supera EDepSim::edepsim )
#target_link_libraries( Supera PUBLIC ${Geant4_LIBRARIES})
#target_include_directories(Supera PUBLIC /app/edep/include)
#target_include_directories(Supera PUBLIC ${PYTHON_INCLUDE_DIRS})
#target_link_libraries(Supera ${PYTHON_LIBRARIES})
#target_include_directories(Supera PUBLIC /app/geant4/include/Geant4)
#target_include_directories(Supera PUBLIC /app/root/include)


#target_include_directories(Supera PRIVATE /app/larcv3/src/larcv3/app/Supera)
#




install (FILES ${HEADERS}
    DESTINATION ${CMAKE_PACKAGE_DIR}/include/larcv3/app/${name})
