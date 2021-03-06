# Copyright 2016 Samsung Electronics Co., Ltd.
# Copyright 2016 University of Szeged.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 2.8.12)
project (Unittest C)

if(NOT FEATURE_PROFILE STREQUAL "full")
  message(FATAL_ERROR "FEATURE_PROFILE='${FEATURE_PROFILE}' doesn't supported with UNITTESTS=ON option")
endif()

# Unit tests main modules
file(GLOB SOURCE_UNIT_TEST_MAIN_MODULES *.c)

# Unit tests declaration
add_custom_target(unittests)

foreach(SOURCE_UNIT_TEST_MAIN ${SOURCE_UNIT_TEST_MAIN_MODULES})
  get_filename_component(TARGET_NAME ${SOURCE_UNIT_TEST_MAIN} NAME_WE)
  set(TARGET_NAME unit-${TARGET_NAME})

  add_executable(${TARGET_NAME} ${SOURCE_UNIT_TEST_MAIN})
  set_property(TARGET ${TARGET_NAME}
               PROPERTY LINK_FLAGS "${LINKER_FLAGS_COMMON}")

  link_directories(${CMAKE_BINARY_DIR})

  set(JERRY_LIBS jerry-core)

  if(JERRY_LIBM)
    set(JERRY_LIBS ${JERRY_LIBS} jerry-libm)
  endif()

  if(JERRY_LIBC)
    set(JERRY_LIBS ${JERRY_LIBS} jerry-libc)
  endif()

  set(JERRY_LIBS ${JERRY_LIBS} ${IMPORTED_LIB})

  target_link_libraries(${TARGET_NAME} ${JERRY_LIBS})

  add_dependencies(unittests ${TARGET_NAME})
endforeach()
