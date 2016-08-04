#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: Split GMSK Modulator
# Generated: Fri Oct 25 13:53:20 2013
##################################################

from gnuradio import analog
from gnuradio import blocks
from gnuradio import digital
from gnuradio import filter
from gnuradio import gr
from gnuradio.filter import firdes
import math

class split_modulator(gr.hier_block2):

	def __init__(self, gain=1):
		gr.hier_block2.__init__(
			self, "Split GMSK Modulator",
			gr.io_signature(0, 0, 0),
			gr.io_signature(1, 1, gr.sizeof_float*1),
		)

		##################################################
		# Parameters
		##################################################
		self.gain = gain

		##################################################
		# Blocks
		##################################################
		self.rational_resampler_xxx_0 = filter.rational_resampler_fff(
		        interpolation=1,
		        decimation=2,
		        taps=(1, ),
		        fractional_bw=None,
		)
		self.pad_source_1 = None;self.message_port_register_hier_out("9k6")
		self.pad_source_0 = None;self.message_port_register_hier_out("4k8")
		self.digital_gmskmod_bc_0_0 = digital.gmskmod_bc(10, 3, 0.3)
		self.digital_gmskmod_bc_0 = digital.gmskmod_bc(10, 3, 0.3)
		self.blocks_pdu_to_tagged_stream_0_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "packet_len")
		self.blocks_pdu_to_tagged_stream_0 = blocks.pdu_to_tagged_stream(blocks.byte_t, "packet_len")
		self.blocks_add_xx_0 = blocks.add_vff(1)
		self.analog_quadrature_demod_cf_0_0 = analog.quadrature_demod_cf(gain)
		self.analog_quadrature_demod_cf_0 = analog.quadrature_demod_cf(gain)

		##################################################
		# Connections
		##################################################
		self.connect((self.blocks_add_xx_0, 0), (self, 0))
		self.connect((self.analog_quadrature_demod_cf_0_0, 0), (self.blocks_add_xx_0, 0))
		self.connect((self.digital_gmskmod_bc_0_0, 0), (self.analog_quadrature_demod_cf_0_0, 0))
		self.connect((self.blocks_pdu_to_tagged_stream_0, 0), (self.digital_gmskmod_bc_0_0, 0))
		self.connect((self.analog_quadrature_demod_cf_0, 0), (self.rational_resampler_xxx_0, 0))
		self.connect((self.rational_resampler_xxx_0, 0), (self.blocks_add_xx_0, 1))
		self.connect((self.digital_gmskmod_bc_0, 0), (self.analog_quadrature_demod_cf_0, 0))
		self.connect((self.blocks_pdu_to_tagged_stream_0_0, 0), (self.digital_gmskmod_bc_0, 0))

		##################################################
		# Asynch Message Connections
		##################################################
		self.msg_connect(self, "4k8", self.blocks_pdu_to_tagged_stream_0, "pdus")
		self.msg_connect(self, "9k6", self.blocks_pdu_to_tagged_stream_0_0, "pdus")

# QT sink close method reimplementation

	def get_gain(self):
		return self.gain

	def set_gain(self, gain):
		self.gain = gain
		self.analog_quadrature_demod_cf_0_0.set_gain(self.gain)
		self.analog_quadrature_demod_cf_0.set_gain(self.gain)


