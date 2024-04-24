"""Serial flashing using Intel HEX over USB STDIO
    @ref https://github.com/vha3/Hunter-Adams-RP2040-Demos/tree/master/Bootloaders/Serial_bootloader
"""
import time
import serial


def serial_output(port: str, hex_file: str):
    """Flash to a Serial port given the port name and hex file

    Args:
        port (str): Port name of the device to flash
        hex_file (str): Path to the compiled .hex file
    """

    # Serial port configurations
    ser = serial.Serial(
        port=port,
        baudrate=9600,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=None
    )

    ser.write(b'DEBUG')

    time.sleep(2.2)
    
    ser.close()
    ser.open()

    # Send some hexlines with bad checksums to get into known bootloader state
    for _ in range(10):
        ser.write(b':020000041000EB\r')
        time.sleep(0.1)
        ser.read(1)

    # We've accumulated junk in the UART input buffer, clear it
    ser.reset_input_buffer()

    # Send one final bad checksum, which puts a single char in our
    # UART input buffer
    ser.write(b':020000041000EB\r')
    time.sleep(0.1)

    with open(hex_file, 'rb') as file:
        lines = file.readlines()
        total_lines = len(lines)

        for line_no, line in enumerate(lines):
            while ser.read(1) == b'\0':
                print(f"{((line_no / total_lines) * 100):.2f}% {line}{' '*10}", end="\r")
                ser.write(line)
            time.sleep(0.01)

    # let's wait one second before reading output (let's give device time to answer)
    time.sleep(1)
    while ser.inWaiting() > 0:
        print(ser.read(1))
    ser.close()


try:
    serial_output('COM12', './build/PMPI.hex')
except serial.serialutil.SerialException as pe:
    print(f"SERIAL FAILED{' '*50}")
