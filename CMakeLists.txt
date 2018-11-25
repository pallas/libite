cmake_minimum_required(VERSION 3.1)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

project(libite)
add_library(libite INTERFACE)
add_executable(argsort argsort.cc)
add_executable(bagofwords bagofwords.cc)
add_executable(dijkstra dijkstra.cc)
add_executable(heapsort heapsort.cc)
add_executable(kruskal kruskal.cc)
add_executable(mergesort mergesort.cc)
add_executable(misra-gries misra-gries.cc)
add_executable(tarjan tarjan.cc)
add_executable(treesort treesort.cc)

file(GLOB header_files "*.h")
install(FILES ${header_files}
	DESTINATION "include/lite"
	)

install(TARGETS
	argsort
	bagofwords
	dijkstra
	heapsort
	kruskal
	mergesort
	misra-gries
	tarjan
	treesort
	RUNTIME DESTINATION bin
	)

enable_testing()
add_executable(test_associative test_associative.cc)
add_test(NAME test_associative COMMAND $<TARGET_FILE:test_associative>)
add_executable(test_intrusive test_intrusive.cc)
add_test(NAME test_intrusive COMMAND $<TARGET_FILE:test_intrusive>)
add_executable(test_list test_list.cc)
add_test(NAME test_list COMMAND $<TARGET_FILE:test_list>)
add_executable(test_queue test_queue.cc)
add_test(NAME test_queue COMMAND $<TARGET_FILE:test_queue>)
