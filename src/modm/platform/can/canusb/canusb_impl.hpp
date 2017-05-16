// coding: utf-8
/* Copyright (c) 2009, Roboterclub Aachen e.V.
 * All Rights Reserved.
 *
 * The file is part of the modm library and is released under the 3-clause BSD
 * license. See the file `LICENSE` for the full license governing this code.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_CAN_USB_HPP
#error "Do not include this file directly. Include canusb.hpp"
#endif

#include <iostream>
#include <modm/debug/logger.hpp>

#include <modm/architecture/driver.hpp>
#include <modm/processing/timer.hpp>

#include <modm/driver/can/can_lawicel_formatter.hpp>
#include "canusb.hpp"

#undef  MODM_LOG_LEVEL
#define MODM_LOG_LEVEL modm::log::INFO

template <typename SerialPort>
modm::hosted::CanUsb<SerialPort>::CanUsb(SerialPort& serialPort)
:	active(false), busState(BusState::Off), serialPort(serialPort)
{
}

template <typename SerialPort>
modm::hosted::CanUsb<SerialPort>::~CanUsb()
{
	if (this->active)
	{
		{
			MutexGuard stateGuard(this->stateLock);
			this->active = false;
		}
		this->thread->join();
		delete this->thread;
		this->thread = 0;
	}
}

template <typename SerialPort>
bool
modm::hosted::CanUsb<SerialPort>::open(modm::Can::Bitrate canBitrate)
{
	if (this->serialPort.open())
	{
		MODM_LOG_DEBUG << MODM_FILE_INFO << "SerialPort opened in canusb" << modm::endl;
		MODM_LOG_DEBUG << MODM_FILE_INFO << "write 'C'" << modm::endl;
		this->serialPort.write("C\r");

		modm::ShortTimeout timeout;
		timeout.restart(500);
		while (not timeout.isExpired())
		{
		}

		char a;
		while( this->serialPort.read(a) );

		// Set CAN bitrate
		MODM_LOG_DEBUG << MODM_FILE_INFO << "Set CAN bitrate" << modm::endl;
		
		switch (canBitrate)
		{
			case kBps10:
				this->serialPort.write("S0\r");
			break;
			case kBps20:
				this->serialPort.write("S1\r");
			break;
			case kBps50:
				this->serialPort.write("S2\r");
			break;
			case kBps100:
				this->serialPort.write("S3\r");
			break;
			case kBps125:
				this->serialPort.write("S4\r");
			break;
			case kBps250:
				this->serialPort.write("S5\r");
			break;
			case kBps500:
				this->serialPort.write("S6\r");
			break;
			case MBps1:
				this->serialPort.write("S8\r");
			break;
		}

		

		timeout.restart(500);
		while (not this->serialPort.read(a))
		{
			if (timeout.isExpired())
			{
				MODM_LOG_DEBUG << MODM_FILE_INFO << "Timer expired" << modm::endl;
				return false;
			}
		}
		if (a != '\r')
		{
			MODM_LOG_ERROR << MODM_FILE_INFO << "Wrong answer on set CAN bitrate: " << modm::hex << (int) a	<< modm::endl;
			return false;
		}
		
		// Open CAN channel
		this->serialPort.write("O\r");
		MODM_LOG_DEBUG << MODM_FILE_INFO << "written 'O'" << modm::endl;
		timeout.restart(500);
		while (not this->serialPort.read(a))
		{
			if (timeout.isExpired())
			{
				MODM_LOG_DEBUG << MODM_FILE_INFO << "Timer expired" << modm::endl;
				return false;
			}
		}

		if (a != '\r')
		{
			MODM_LOG_ERROR << MODM_FILE_INFO << "Wrong answer on O: " << modm::hex << (int) a << modm::endl;
			return false;
		}

		{
			MutexGuard stateGuard(this->stateLock);
			this->active = true;
		}
		this->thread = new boost::thread(
				boost::bind(&modm::hosted::CanUsb<SerialPort>::update, this));

		busState = BusState::Connected;
		this->tmpRead.clear();
		return true;
	}
	else
	{
		busState = BusState::Off;
		MODM_LOG_ERROR << MODM_FILE_INFO << "Could not open Canusb" << modm::endl;
		return false;
	}
}

template <typename SerialPort>
void
modm::hosted::CanUsb<SerialPort>::close()
{
	this->serialPort.write("C\r");
	{
		MutexGuard stateGuard(this->stateLock);
		this->active = false;
	}

	this->thread->join();
	delete this->thread;
	this->thread = 0;
	busState = BusState::Off;
}

template <typename SerialPort>
modm::Can::BusState
modm::hosted::CanUsb<SerialPort>::getBusState()
{
	return busState;
}

template <typename SerialPort>
bool
modm::hosted::CanUsb<SerialPort>::getMessage(can::Message& message)
{
	if (not this->readBuffer.empty())
	{
		message = this->readBuffer.front();
		this->readBuffer.pop();
		return true;
	}
	else
	{
		return false;
	}
}

template <typename SerialPort>
bool
modm::hosted::CanUsb<SerialPort>::sendMessage(const can::Message& message)
{
	char str[128];
	modm::CanLawicelFormatter::convertToString(message, str);
	MODM_LOG_DEBUG.printf("Sending ");
	char *p = str;
	while (*p != '\0') {
		MODM_LOG_DEBUG.printf("%02x %c, ", *p, *p);
		++p;
	}
	this->serialPort.write(str);
	this->serialPort.write("\r");
	return true;
}

template <typename SerialPort>
void
modm::hosted::CanUsb<SerialPort>::update()
{
	while (true)
	{
		char a;
		if (this->serialPort.read(a))
		{
			MODM_LOG_DEBUG.printf("Received %02x\n", a);
			if (a == 'T' || a == 't' || a == 'r' || a == 'R')
			{
				this->tmpRead.clear();
			}
			this->tmpRead += a;

			can::Message message;
			if (modm::CanLawicelFormatter::convertToCanMessage(
					this->tmpRead.c_str(), message))
			{
				this->readBuffer.push(message);
			}
		}
		if (not this->active) {
			break;
		}
	}
}