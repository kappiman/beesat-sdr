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
 */

/*
 * frame_composer.h
 *
 *  Created on: 16.10.2013
 *      Author: Philipp Werner
 */


#ifndef INCLUDED_TNC_NX_FRAME_COMPOSER_H
#define INCLUDED_TNC_NX_FRAME_COMPOSER_H

#include <tnc_nx/api.h>
#include <tnc_nx/mobitex_coding.h>

#include <stdint.h>
#include <cstdio>

namespace gr {
  namespace tnc_nx {

    class TNC_NX_API frame_composer
    {
    private:
    	TNC_NX_API mobitex_coding *mob;

    	// conversion from binary to M-ary +/- 1
    	int8_t con(uint8_t bit);


    public:
    	frame_composer(mobitex_coding* mob_ptr);
    	~frame_composer();

    	int write_sync(int8_t *buf);
    	int write_header(int8_t *buf);
    	int write_callsign(int8_t *buf);
    	int write_data(int8_t *buf);
    	int write_ec(int8_t *buf);
    	int write_sdb(int8_t *buf);
    	int write_hangbytes(int8_t *buf);
    	int write_baud_sync(int8_t *buf_1, int8_t *buf_2);
        int write_bc_header(int8_t *buf);

        bool read_ctrl(uint8_t bit);
        bool read_ctrl_fec(uint8_t bit);
        bool read_callsign(uint8_t bit);
        bool read_data(uint8_t bit);
        bool read_ec(uint8_t bit);
        bool read_sdb(uint8_t bit);
        bool skip_fh();



    };

  } // namespace tnc_nx
} // namespace gr

#endif /* INCLUDED_TNC_NX_FRAME_COMPOSER_H */

