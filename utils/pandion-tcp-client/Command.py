import struct

class Command:
    def __init__(self, str_name, code):
            self.str_name = str_name
            self.code = code

    def code_to_bytes(self):
        ba = bytearray()
        ba.append(self.code & 0xFF)
        ba.append(self.code >> 8)
        return ba

    def to_bytes(self, value):
        return self.code_to_bytes()

class FloatGetCommand(Command):
    pass

class FloatSetCommand(Command):
    def to_bytes(self, value):
        ba = self.code_to_bytes()
        ba.extend(struct.pack("f", float(value)))

        return ba