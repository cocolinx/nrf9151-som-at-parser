# CocoLinx nRF9151-SOM AT command parser Library

## Description

A C based AT command parser library for CocoLinx nRF9151-SOM.

## Features

- Network attach / detach control
- UDP / TCP / MQTT / secure UDP / secure TCP / secure MQTT / RS485 data transmission
- GNSS positioning support

## Porting

Before using this library, implement the platform-dependent functions for your target MCU or platform.

The following functions must be implemented according to your UART driver and system timer:

- `uart_write()`
- `uart_read()`
- `uart_flush()`
- `get_ms()`
- `sleep_ms()`

If required, add your target-specific UART initialization code to `begin()`.


## License

This project is primarily licensed under the MIT License.

Some hexadecimal conversion utility code is derived from the Zephyr Project and is licensed under the Apache License 2.0.

See LICENSES for details.