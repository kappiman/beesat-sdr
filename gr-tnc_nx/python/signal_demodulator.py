#!/usr/bin/env python
##################################################
# Gnuradio Python Flow Graph
# Title: GMSK Demodulator
# Generated: Wed Oct 30 15:58:44 2013
##################################################

from gnuradio import blocks
from gnuradio import digital
from gnuradio import filter
from gnuradio import gr
from gnuradio.filter import firdes

class signal_demodulator(gr.hier_block2):

	def __init__(self, dc_block_len=1000, samp_per_sym=10, rx_bit_inv=1):
		gr.hier_block2.__init__(
			self, "GMSK Demodulator",
			gr.io_signature(1, 1, gr.sizeof_float*1),
			gr.io_signature(1, 1, gr.sizeof_char*1),
		)

		##################################################
		# Parameters
		##################################################
		self.dc_block_len = dc_block_len
		self.samp_per_sym = samp_per_sym
		self.rx_bit_inv = rx_bit_inv

		##################################################
		# Variables
		##################################################
		self.samp_rate = samp_rate = 48000

		##################################################
		# Blocks
		##################################################
		self.digital_clock_recovery_mm_xx_0_0 = digital.clock_recovery_mm_ff(samp_per_sym*(1+0.0), 0.25*0.175*0.175, 0.5, 0.175, 0.005)
		self.digital_binary_slicer_fb_0_0 = digital.binary_slicer_fb()
		self.dc_blocker_xx_0 = filter.dc_blocker_ff(dc_block_len, False)
		self.blocks_multiply_const_vxx_0_0 = blocks.multiply_const_vff((rx_bit_inv, ))

		##################################################
		# Connections
		##################################################
		self.connect((self.dc_blocker_xx_0, 0), (self.blocks_multiply_const_vxx_0_0, 0))
		self.connect((self.digital_clock_recovery_mm_xx_0_0, 0), (self.digital_binary_slicer_fb_0_0, 0))
		self.connect((self.blocks_multiply_const_vxx_0_0, 0), (self.digital_clock_recovery_mm_xx_0_0, 0))
		self.connect((self.digital_binary_slicer_fb_0_0, 0), (self, 0))
		self.connect((self, 0), (self.dc_blocker_xx_0, 0))


# QT sink close method reimplementation

	def get_dc_block_len(self):
		return self.dc_block_len

	def set_dc_block_len(self, dc_block_len):
		self.dc_block_len = dc_block_len

	def get_samp_per_sym(self):
		return self.samp_per_sym

	def set_samp_per_sym(self, samp_per_sym):
		self.samp_per_sym = samp_per_sym
		self.digital_clock_recovery_mm_xx_0_0.set_omega(self.samp_per_sym*(1+0.0))

	def get_rx_bit_inv(self):
		return self.rx_bit_inv

	def set_rx_bit_inv(self, rx_bit_inv):
		self.rx_bit_inv = rx_bit_inv
		self.blocks_multiply_const_vxx_0_0.set_k((self.rx_bit_inv, ))

	def get_samp_rate(self):
		return self.samp_rate

	def set_samp_rate(self, samp_rate):
		self.samp_rate = samp_rate


