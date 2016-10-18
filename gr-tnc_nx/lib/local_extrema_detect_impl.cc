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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "local_extrema_detect_impl.h"

namespace gr {
  namespace tnc_nx {

    local_extrema_detect::sptr
    local_extrema_detect::make(int hist, float threshold, float gain)
    {
      return gnuradio::get_initial_sptr
        (new local_extrema_detect_impl(hist, threshold, gain));
    }

    // last output value
    float _rem = 0;

    /*
     * The private constructor
     */
    local_extrema_detect_impl::local_extrema_detect_impl(int hist, float threshold, float range)
      : gr::block("local_extrema_detect",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(float))),
              _hist(hist),
              _threshold(threshold),
              _gain(range)
    {
    	set_history(_hist+1);
    }

    /*
     * Our virtual destructor.
     */
    local_extrema_detect_impl::~local_extrema_detect_impl()
    {
    }



    int
    local_extrema_detect_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        float *out = (float *) output_items[0];

        float diff = 0;

        /*
         * "local_extrema detector algorithm"
         */
        for(int i=_hist; i<(noutput_items+_hist); ++i){
            if (in[i] > _threshold || in[i] < -_threshold) {
                out[i] = in[i];
                continue;
            }

            // set the inital difference
            diff = in[i-_hist+1] - in[i-_hist];
            bool foundLocalExtrema = false;
            for (int j = _hist; j > 0; --j) {
                float slope = in[i-j+1] - in[i-j];

                if (slope > 0 && diff < 0) {
                    foundLocalExtrema = true;
                    _rem = _gain;
                }
                else if (slope < 0 && diff > 0){
                    foundLocalExtrema = true;
                    _rem = -_gain;
                }
                diff = slope;
               
               
            }
            if (!foundLocalExtrema) {
                out[i] = in[i];
                continue;
            }
            for (int j = 0; j < _hist; ++j) {
                out[i - j] = _rem;
            }
        }

        // Tell runtime system how many input items we consumed on
        // each input stream.
        consume_each (noutput_items);

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace tnc_nx */
} /* namespace gr */

