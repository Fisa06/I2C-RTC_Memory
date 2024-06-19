import serial
import serial.tools.list_ports
import ntplib
from datetime import datetime, timezone, timedelta


def int_to_bcd(n):
    return (n // 10) << 4 | (n % 10)


def get_ntp_time():
    c = ntplib.NTPClient()
    response = c.request('pool.ntp.org')
    dt = datetime.fromtimestamp(response.tx_time, timezone.utc)
    dt = dt + timedelta(hours=2)

    hours_bcd = int_to_bcd(dt.hour)
    minutes_bcd = int_to_bcd(dt.minute)
    seconds_bcd = int_to_bcd(dt.second)
    print(f"Hours: {hex(hours_bcd)[2:]}, Minutes: {hex(minutes_bcd)[2:]}, Seconds: {hex(seconds_bcd)[2:]}")

    return hours_bcd, minutes_bcd, seconds_bcd


def find_microcontroller_port():
    ports = list(serial.tools.list_ports.comports())
    for p in ports:
        print(p.description)
        if "STM" in p.description:
            return p.device
    return None


def send_time_to_microcontroller(lol=0):
    port = find_microcontroller_port()
    print(port)
    if port is None:
        print("Microcontroller not found")
        print(list(serial.tools.list_ports.comports()))
        return
    if lol:
        hours_bcd, minutes_bcd, seconds_bcd = get_ntp_time()
        print(f"Sending time to microcontroller: {hours_bcd}, {minutes_bcd}, {seconds_bcd}")
                                                               
        with serial.Serial(port, 115200, timeout=1) as ser:
            ser.write(bytes([0xff, hours_bcd, minutes_bcd, seconds_bcd]))
 #testmode
    else:
        with serial.Serial(port, 115200, timeout=1) as ser:
            ser.write(bytes([0xff, 0x10, 0x00, 0x11]))


send_time_to_microcontroller(1)
