import socket
from Command import Command, FloatGetCommand, FloatSetCommand

SERVER_IP = '192.168.4.1'
SERVER_PORT = 3000

COMMANDS = [
    FloatSetCommand('set_pitch_p', 0x0110),
    FloatSetCommand('set_pitch_i', 0x0111),
    FloatSetCommand('set_pitch_d', 0x0112),
    FloatGetCommand('get_pitch_p', 0x0113),
    FloatGetCommand('get_pitch_i', 0x0114),
    FloatGetCommand('get_pitch_d', 0x0115),
    FloatSetCommand('set_roll_p', 0x0116),
    FloatSetCommand('set_roll_i', 0x0117),
    FloatSetCommand('set_roll_d', 0x0128),
    FloatGetCommand('get_roll_p', 0x0119),
    FloatGetCommand('get_roll_i', 0x011a),
    FloatGetCommand('get_roll_d', 0x011b),
    FloatSetCommand('set_yaw_p', 0x011c),
    FloatSetCommand('set_yaw_i', 0x011d),
    FloatSetCommand('set_yaw_d', 0x011e),
    FloatGetCommand('get_yaw_p', 0x011d),
    FloatGetCommand('get_yaw_i', 0x011e),
    FloatGetCommand('get_yaw_d', 0x011f)
]


def connect():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.settimeout(5)

    try:
        sock.connect((SERVER_IP, SERVER_PORT))
    except socket.timeout:
        print("Connection timed out")
        exit(0)

    print("Connected!")
    return sock

def parse_command(raw_command):
    tokens = raw_command.split(' ')
    
    if len(tokens) > 2:
        return None

    value = None if len(tokens) == 1 else tokens[1]

    for command in COMMANDS:    
        if command.str_name == tokens[0]:            
            if isinstance(command, FloatGetCommand):
                return command.to_bytes(value)
            elif isinstance(command, FloatSetCommand):
                return command.to_bytes(value)
    
    return None



raw_command = input("Input Command >")

tx_bytes = parse_command(raw_command)

if tx_bytes == None:
    print("invalid command")
else:
    print([ "0x%02x" % b for b in tx_bytes ])



bytesOut = bytearray()
bytesOut.append(0x03)
bytesOut.append(0x01)
bytesOut.append(0x03)
bytesOut.append(0x04)

# sock.send(bytesOut)
# response = sock.recv(4)
# sock.close()

print("Done!")
# print("received data: ", response)