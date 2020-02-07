# ConradRelay_arduino
Code for Arduino (nano in my case) to work as Conrad Relay Card

Python example: 

```python
import serial

ser = serial.Serial('COM1', 9600)
ser.timeout = 4

def cmd(com, data):
    b = bytearray([com&0xFF, 0, data&0xFF, 0])
    b[3] = b[0] ^ b[1] ^ b[2]
    ser.write(b)
    print(ser.read(4))

print('init')
cmd(1, 0)
        
print('set all')
cmd(3, 0xFF)

print('reset all')
cmd(3, 0)

print('toggle pin D2')
cmd(8, 1)

```

## Pin usage 

Count starts from GPIO D2 

## Relay

[Elegoo Relay Module DC 5V with Optocoupler for Arduino](https://www.amazon.de/gp/product/B01M8G4Y7Z/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)

## Arduino

[Elegoo Compatible Nano Board for Arduino](https://www.amazon.de/gp/product/B0713ZRJLC/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1)

### issues 

- reboots with each serial port reconnection, to fix remove a capactior, connected to RST pin (DTR-RST)

