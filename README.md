# KSP-Custom-Controller

This is a custom built game controller with 12 buttons and 7 analog axis using a teensy 4.0 and a 3D-printed housing designed for Kerbal Space Program. It's heavily inspired by [ksp-controller](https://github.com/malarcky/ksp-controller/) by *malarcky*.

## Setting up the custom controller

The teensy 4.0 micro controller is programmed using Arduino IDE and a custom USB type. To create the custom USB type, changes are made to the following files found in the teensy package. For me on windows, these files are located in `%appdata%\Local\Arduino15\packages\teensy\hardware\avr\1.59.0\cores\teensy4`.

### usb_desc.h

This code sets up the USB type and can be inserted any where after the first `#if defined()`:
```
#elif defined(USB_CUSTOM_CONTROLLER)
	#define VENDOR_ID 0x16C0
	#define PRODUCT_ID 0x0488
	#define BCD_DEVICE 0x0211
	#define MANUFACTURER_NAME {'T','h','r','e','e','P','o','u','n','d','s'}
	#define MANUFACTURER_NAME_LEN 11
	#define PRODUCT_NAME {'C','u','s','t','o','m',' ','K','S','P',' ','C','o','n','t','r','o','l','l','e','r'}
	#define PRODUCT_NAME_LEN 21
	#define EP0_SIZE 64
	#define NUM_ENDPOINTS 3
	#define NUM_INTERFACE 2
	#define SEREMU_INTERFACE 1 // Serial emulation
	#define SEREMU_TX_ENDPOINT 2
	#define SEREMU_TX_SIZE 64
	#define SEREMU_TX_INTERVAL 1
	#define SEREMU_RX_ENDPOINT 2
	#define SEREMU_RX_SIZE 32
	#define SEREMU_RX_INTERVAL 2
	#define JOYSTICK_INTERFACE 2 // Joystick
	#define JOYSTICK_ENDPOINT 3
	#define JOYSTICK_SIZE 16 // 16 = custom game controller, 12 = normal joystick, 64 = extreme joystick
	#define JOYSTICK_INTERVAL 1
	#define ENDPOINT2_CONFIG ENDPOINT_RECEIVE_INTERRUPT + ENDPOINT_TRANSMIT_INTERRUPT
	#define ENDPOINT3_CONFIG ENDPOINT_RECEIVE_UNUSED + ENDPOINT_TRANSMIT_INTERRUPT
```

### usb_joystick.h

This handles this axis and button assignment and must be added before the line `#elif JOYSTICK_SIZE == 64`.

```
#elif JOYSTICK_SIZE == 16 // Modified for 7-Axis, 12-Button, 16-Byte Report
	/**
	 * Sets the state of a button (1-12).
	 */
	void button(unsigned int num, bool val) {
		if (--num >= 12) return;
		// The 12 buttons fit in the first 16-bit word of the report.
		uint16_t *p = (uint16_t *)usb_joystick_data;
		if (val) {
			*p |= (1 << num);
		} else {
			*p &= ~(1 << num);
		}
		if (!manual_mode) usb_joystick_send();
	}

	// Axis Functions (public interface remains the same)
	void X(unsigned int position) { analog16(0, position); }
	void Y(unsigned int position) { analog16(1, position); }
	void Z(unsigned int position) { analog16(2, position); }
	void Xrotate(unsigned int position) { analog16(3, position); }
	void Yrotate(unsigned int position) { analog16(4, position); }
	void Zrotate(unsigned int position) { analog16(5, position); }

	/**
	 * Sets the position of the 7th axis (Slider).
	 * The original 'num' parameter is removed as we only have one slider.
	 */
	void slider(unsigned int position) {
		analog16(6, position);
	}
```

In the `private:` section before the line `#if JOYSTICK_SIZE == 64` add:

```
#if JOYSTICK_SIZE == 16
	void analog16(unsigned int num, unsigned int value) {
		if (num >= 7) return; // Safety check for 7 axes
		if (value > 1023) value = 1023;
		uint16_t scaled_value = (value << 6) | (value >> 4);
		
		uint16_t *p = (uint16_t *)usb_joystick_data;
		
		p[num + 1] = scaled_value;

		if (!manual_mode) usb_joystick_send(); 
	}
#endif
```

## 3D-printed housing

The housing was modelled using FreeCad 1.0.2. Included are .3mf files for the face plate and housing so they can be printed separately.

## The finished controller

![image](https://github.com/ThreePounds/KSP-Custom-Controller/blob/main/img/finished-controller.jpg "the finished controller sitting on a desk")
