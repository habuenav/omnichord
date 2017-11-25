input = ARGV[0]
output = ARGV[1]

s = File.binread(input)
bytes = s.unpack("C*")
code  = "const prog_uchar SAMPLE[] PROGMEM = {#{bytes.join(', ')}};\n"
code += "const unsigned int SAMPLE_SIZE = #{bytes.size};"

File.write(output, code)
