set(cxx_default "${CMAKE_CXX_FLAGS}")

# cxx_executable_with_flags(name cxx_flags libs testlib srcs...)
#
# creates a named C++ executable that depends on the given libraries and
# is built from the given source files with the given compiler flags.
function(cxx_executable_with_flags name cxx_flags libs testlib)
  add_executable(${name} ${ARGN})
  if (cxx_flags)
    set_target_properties(${name}
      PROPERTIES
      COMPILE_FLAGS "${cxx_flags}")
  endif()
  # To support mixing linking in static and dynamic libraries, link each
  # library in with an extra call to target_link_libraries.
  foreach (lib "${libs}")
    target_link_libraries(${name} ${lib})
  endforeach()

  add_dependencies(${name} "${testlib}")
  target_link_libraries(${name} "${testlib}")
endfunction()

# cxx_executable(name dir lib testlib srcs...)
#
# creates a named target that depends on the given libs and is built
# from the given source files.  dir/name.cc is implicitly included in
# the source file list.
function(cxx_executable name dir libs testlib)
  cxx_executable_with_flags(
    ${name} "${cxx_default}" "${libs}" "${dir}/${name}.cc" "${testlib}" ${ARGN})
endfunction()

# cxx_test_with_flags(name cxx_flags libs testlib srcs...)
#
# creates a named C++ test that depends on the given libs and is built
# from the given source files with the given compiler flags.
function(cxx_test_with_flags name cxx_flags libs testlib)
  cxx_executable_with_flags(${name} "${cxx_flags}" "${libs}" "${testlib}" ${ARGN})
  add_test(${name} ${name})
endfunction()

# cxx_test(name libs testlib srcs...)
#
# creates a named test target that depends on the given libs and is
# built from the given source files.  Unlike cxx_test_with_flags,
# test/name.cc is already implicitly included in the source file list.
function(cxx_test name libs testlib)
  cxx_test_with_flags("${name}" "${cxx_default}" "${libs}" "${testlib}"
    "../test/src/${name}.cc" ${ARGN})
endfunction()
