/* This file is part of DoIP. DoIP is free software: you 
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later 
 * version.
 * 
 * DoIP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with DoIP.  If not, see <http://www.gnu.org/licenses/>. */
 
 #include "../include/ambxleds.h"

#include <TJShared/include/tjshared.h>
#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef TJ_OS_LINUX
	#define HAVE_LIBUSB
#endif

#ifdef HAVE_LIBUSB
	#include <usb.h>
#else
	#warning Ambx LEDs not supported on this platform, need libusb
#endif

using namespace std;
using namespace tj::shared;
using namespace tj::ep::leds;

bool AmbxLED::_usbInitialized = false;

void AmbxLED::Initialize() {
	#ifdef HAVE_LIBUSB
		if(!_usbInitialized) {
			_usbInitialized = true;
			usb_init();
			usb_find_busses();
			usb_find_devices();
		}
	#endif
}

AmbxLED::AmbxLED(): _device(0), _r(0), _g(0), _b(0) {
	#ifdef HAVE_LIBUSB
		Initialize();
	
		for (struct usb_bus* bus = usb_busses; bus; bus = bus->next) {
			for (struct usb_device* dev = bus->devices; dev; dev = dev->next) {
				if ((dev->descriptor.idVendor == 0x0471) && (dev->descriptor.idProduct == 0x083f)) {
					usb_dev_handle* handle = usb_open(dev);
					if(handle==NULL) {
						Throw(L"Could not open handle to device", ExceptionTypeError);
					}
					
					if(usb_set_configuration(handle, 1)!=0) {
						Throw(L"Could not set configuration", ExceptionTypeError);
					}
					
					if(usb_claim_interface(handle,0)!=0) {
						Throw(L"Could not claim interface", ExceptionTypeError);
					}
					
					_device = reinterpret_cast<void*> (handle);
					break;
				}
			}
		}
	#endif
}

AmbxLED::~AmbxLED() {
	#ifdef HAVE_LIBUSB
		usb_dev_handle* handle = reinterpret_cast<usb_dev_handle*>(_device);
		if(handle!=0) {
			usb_release_interface(handle,0);
			usb_close(handle);
		}
	#endif
}

void AmbxLED::SetColor(Light l, unsigned char r, unsigned char g, unsigned char b) {
	#ifdef HAVE_LIBUSB
		usb_dev_handle* handle = reinterpret_cast<usb_dev_handle*>(_device);
		if(handle!=0) {
			unsigned char data[] = {0x01, (unsigned char)l, 0x03, r, g, b};
			int n = usb_bulk_write(handle, 0x02, (char*)data, 6, 10);
			if(n<=0) {
				Throw(L"Could not write data to lights", ExceptionTypeError);
			}
			_r = r; _g = g; _b = b;
		}
		else {
			Throw(L"No device opened, cannot send command", ExceptionTypeError);
		}
	#endif
}

void AmbxLED::SetColorFading(Light l, unsigned char r, unsigned char g, unsigned char b) {
#ifdef HAVE_LIBUSB
	usb_dev_handle* handle = reinterpret_cast<usb_dev_handle*>(_device);
	if(handle!=0) {
		int fadingTimeMS = 1000;
		int delay = fadingTimeMS/16;
		
		unsigned char data[53];
		data[0] = 0x01;
		data[1] = l;
		data[2] = 0x72;
		data[3] = (delay & 0xFF00) >> 8;
		data[4] = (delay & 0xFF);
		
		for(int a=0;a<15;a++) {
			float fraction = float(a)/16.0f;
			data[5+(a*3)] = (unsigned char)(float(r-_r)*fraction + _r);
			data[6+(a*3)] = (unsigned char)(float(g-_g)*fraction + _g);
			data[7+(a*3)] = (unsigned char)(float(b-_b)*fraction + _b);
		}

		// Set the last value explicitly to prevent rounding errors that leave the lights on even when fading to (0,0,0)
		data[5+(15*3)] = r;
		data[6+(15*3)] = g;
		data[7+(15*3)] = b;
		
		int n = usb_bulk_write(handle, 0x02, (char*)data, 53, 10);
		if(n<=0) {
			Throw(L"Could not write data to lights", ExceptionTypeError);
		}
		
	}
	else {
		Throw(L"No device opened, cannot send command", ExceptionTypeError);
	}
#endif
}

void AmbxLED::SetColorFading(unsigned char r, unsigned char g, unsigned char b) {	
	SetColorFading(LeftWallLight, r, g, b);
	SetColorFading(RightWallLight, r, g, b);
	SetColorFading(CenterWallLight, r, g, b);
	SetColorFading(LeftSpeakerLight, r, g, b);
	SetColorFading(RightSpeakerLight, r, g, b);
	_r = r; _g = g; _b = b;
}

void AmbxLED::SetColorDirectly(unsigned char r, unsigned char g, unsigned char b) {	
	SetColor(LeftWallLight, r, g, b);
	SetColor(RightWallLight, r, g, b);
	SetColor(CenterWallLight, r, g, b);
	SetColor(LeftSpeakerLight, r, g, b);
	SetColor(RightSpeakerLight, r, g, b);
}