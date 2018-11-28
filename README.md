# ConradRelay_arduino
Code for Arduino to work as Conrad Relay Card

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
