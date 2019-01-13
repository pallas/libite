cmake_minimum_required(VERSION 3.1)

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
		STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
		"Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set (CMAKE_CXX_STANDARD 11)

project(libite)
add_library(libite INTERFACE)

set(${PROJECT_NAME}_EXECUTABLES
	argsort
	bagofwords
	dijkstra
	heapsort
	kruskal
	mergesort
	misra-gries
	tarjan
	treesort
	)

set(${PROJECT_NAME}_HEADERS
	heap
	link
	list
	queue
	set
	stack
	table
	tree
	)

set(${PROJECT_NAME}_TESTS
	test_associative
	test_intrusive
	test_list
	test_queue
	)

foreach(executable ${${PROJECT_NAME}_EXECUTABLES})
	add_executable(${executable} ${executable}.cc)
	install(TARGETS ${executable} RUNTIME DESTINATION bin)
endforeach(executable)

foreach(header ${${PROJECT_NAME}_HEADERS})
	install(FILES ${header}.h DESTINATION "include/lite")
endforeach(header)

enable_testing()
add_custom_target(all_tests)
foreach(test ${${PROJECT_NAME}_TESTS})
	add_executable(${test} EXCLUDE_FROM_ALL ${test}.cc)
	add_test(NAME ${test} COMMAND $<TARGET_FILE:${test}>)
	add_dependencies(all_tests ${test})
endforeach(test)
build_command(CTEST_CUSTOM_PRE_TEST TARGET all_tests)
string(CONFIGURE \"@CTEST_CUSTOM_PRE_TEST@\" CTEST_CUSTOM_PRE_TEST_QUOTED ESCAPE_QUOTES)
file(WRITE "${CMAKE_BINARY_DIR}/CTestCustom.cmake" "set(CTEST_CUSTOM_PRE_TEST ${CTEST_CUSTOM_PRE_TEST_QUOTED})" "\n")

foreach(sorter heapsort mergesort treesort)
	add_test(NAME ${sorter} COMMAND sh -c "
		[ -x '$<TARGET_FILE:${sorter}>' ] &&
		seq 128 |
		sort -R |
		$<TARGET_FILE:${sorter}> |
		sort -c -n
	")
	add_dependencies(all_tests ${sorter})
endforeach(sorter)
