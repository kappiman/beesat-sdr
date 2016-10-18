/* -*- c++ -*- */
/* 
 * Copyright 2013 - 2016 Chair of Space Technology, Technische Universität Berlin
 * 
 * Authors: Philip Werner, Sascha Kapitola
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 * 
 * 
 */

#ifndef INCLUDED_TEST_BLOCKS_SLOPE_DETECT_IMPL_H
#define INCLUDED_TEST_BLOCKS_SLOPE_DETECT_IMPL_H

#include <tnc_nx/local_extrema_detect.h>

namespace gr {
  namespace tnc_nx {

    class local_extrema_detect_impl : public local_extrema_detect
    {
     private:
      int 		_hist;
      float 	_threshold;
      float		_gain;

     public:
      local_extrema_detect_impl(int hist, float gain, float range);
      ~local_extrema_detect_impl();

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);

      int hist() const { return _hist; }
      float threshold() const { return _threshold; }
      float gain() const { return _gain; }
      void set_hist(int hist) { _hist = hist; }
      void set_threshold(float threshold) { _threshold = threshold; }
      void set_gain(float range) { _gain = range; }
    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_LOCAL_EXTREMA_DETECT_IMPL_H */

