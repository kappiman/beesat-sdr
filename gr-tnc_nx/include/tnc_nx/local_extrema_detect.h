/* -*- c++ -*- */
/* 
 * Copyright 2016 <+ Chair of Space technology, TU Berlin +>.
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
 * \author Sascha Kapitola, Philipp Werner
 */


#ifndef INCLUDED_LOCAL_EXTREMA_DETECT_H
#define INCLUDED_LOCAL_EXTREMA_DETECT_H

#include <tnc_nx/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace tnc_nx {

    /*!
     * \brief Detects an extrema in a local area of a signal
     *        The local area is defined by parameter hist.
     *        Signal is only checked if within interval [-threshold, +threshold]
     *        If an extrema is detected the signal is set to gain
     * \ingroup tnc_nx
     *
     */
    class TNC_NX_API local_extrema_detect : virtual public gr::block
    {
     public:
      typedef boost::shared_ptr<local_extrema_detect> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of test_blocks::slope_detect.
       *
       * To avoid accidental use of raw pointers, test_blocks::slope_detect's
       * constructor is in a private implementation
       * class. test_blocks::slope_detect::make is the public interface for
       * creating new instances.
       */
      static sptr make(int hist, float threshold, float gain);

      /**
       * \brief Set the number of samples to analyze
       */
      virtual int hist() const = 0;
      virtual float threshold() const = 0;
      virtual float gain() const = 0;
    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_LOCAL_EXTREMA_DETECT_H */

