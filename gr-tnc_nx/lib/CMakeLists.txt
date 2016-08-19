# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND tnc_nx_sources
    mobitex_coding.cc
    frame_composer.cc
    nx_protocol.cc
    tnc_impl.cc
    local_extrema_detect_impl.cc
    nx_encoder_impl.cc
    nx_decoder_impl.cc
    tnc_b1_impl.cc
    gscf_com.cc
)

set(tnc_nx_sources "${tnc_nx_sources}" PARENT_SCOPE)
if(NOT tnc_nx_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT tnc_nx_sources)

add_library(gnuradio-tnc_nx SHARED ${tnc_nx_sources})
target_link_libraries(gnuradio-tnc_nx ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES})
set_target_properties(gnuradio-tnc_nx PROPERTIES DEFINE_SYMBOL "gnuradio_tnc_nx_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-tnc_nx PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-tnc_nx
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
##include(GrTest)

##include_directories(${CPPUNIT_INCLUDE_DIRS})

##list(APPEND test_tnc_nx_sources
##    ${CMAKE_CURRENT_SOURCE_DIR}/test_tnc_nx.cc
##    ${CMAKE_CURRENT_SOURCE_DIR}/qa_tnc_nx.cc
##)

##add_executable(test-tnc_nx ${test_tnc_nx_sources})

##target_link_libraries(
##  test-tnc_nx
##  ${GNURADIO_RUNTIME_LIBRARIES}
##  ${Boost_LIBRARIES}
##  ${CPPUNIT_LIBRARIES}
##  gnuradio-tnc_nx
##)

##GR_ADD_TEST(test_tnc_nx test-tnc_nx)
