# -*- coding: utf-8 -*-
# 
# Copyright (c) 2013, Roboterclub Aachen e.V.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of the Roboterclub Aachen e.V. nor the
#    names of its contributors may be used to endorse or promote products
#    derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY ROBOTERCLUB AACHEN E.V. ''AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL ROBOTERCLUB AACHEN E.V. BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# -----------------------------------------------------------------------------

from lxml import etree

from reader import XMLDeviceReader

import os, sys
# add python module logger to path
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'logger'))
from logger import Logger

class XMLAggregator:
	""" XMLAggregator
	Base class for all device aggregator which is essentially a glorified XPath wrapper.
	The idea is to smartly compare all devices on a node level, resulting in a compact
	decription of the commonalities and differences between the devices.
	"""

	def __init__(self, devices, logger=None):
		if logger == None:
			self.log = Logger()
		else:
			self.log = logger
		
		self.devices = devices
		self.tree = etree.Element('files')
		# add all device trees into the root view
		for dev in self.devices:
			self.tree.append(dev.tree)

	def applyQuery(self, query):
		result = None
		
		try:
			result = self.tree.xpath(query)
			if len(result) == 0:
				self.log.warn("XMLAggregator: No results found for '" + str(query) + "'")
		except:
			self.log.error("XMLAggregator: Query failed for '" + str(query) + "'")
		
		return result

	def __repr__(self):
		return self.__str__()

	def __str__(self):
		return "XMLAggregator(" +  + ")"
