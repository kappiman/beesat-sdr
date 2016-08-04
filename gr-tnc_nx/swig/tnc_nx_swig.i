/* -*- c++ -*- */

#define TNC_NX_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "tnc_nx_swig_doc.i"

%{
#include "tnc_nx/tnc.h"
#include "tnc_nx/nx_encoder.h"
#include "tnc_nx/nx_decoder.h"
#include "tnc_nx/tnc_b1.h"
#include "tnc_nx/gscf_com.h"
#include "tnc_nx/local_extrema_detect.h"
%}


%include "tnc_nx/tnc.h"
GR_SWIG_BLOCK_MAGIC2(tnc_nx, tnc);
%include "tnc_nx/nx_encoder.h"
GR_SWIG_BLOCK_MAGIC2(tnc_nx, nx_encoder);
%include "tnc_nx/nx_decoder.h"
GR_SWIG_BLOCK_MAGIC2(tnc_nx, nx_decoder);
%include "tnc_nx/tnc_b1.h"
GR_SWIG_BLOCK_MAGIC2(tnc_nx, tnc_b1);
%include "tnc_nx/gscf_com.h"
%include "tnc_nx/local_extrema_detect.h"
GR_SWIG_BLOCK_MAGIC2(tnc_nx, local_extrema_detect);
