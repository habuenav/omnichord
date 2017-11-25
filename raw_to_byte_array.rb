input = ARGV[0]
output = ARGV[1]

s = File.binread(input)
bytes = s.unpack("C*")
code = "byte sample[] = {#{bytes.join(', ')}};"

File.write(output, code)
