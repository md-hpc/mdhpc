def parse_hex_line(line):
    line = line.strip()
    if len(line) < 51:
        raise ValueError(f"Line too short to parse: {line}")

    idx = len(line)
    int_32bit_list = []
    for _ in range(6):
        int_32bit = int(line[idx-8:idx], 16)
        int_32bit_list.append(int_32bit)
        idx -= 8
    # Next 2 characters = 8-bit int
    int_8bit = int(line[idx-2:idx], 16)
    idx -= 2
    # Last 3 characters = 12-bit int
    int_12bit = int(line[idx-3:idx], 16)
    idx -= 3
    # Next 6 * 8 characters = 6 x 32-bit ints (from right to left)
    

    return int_32bit_list, int_8bit, int_12bit

def read_and_parse_file(filename):
    results = []
    with open(filename, 'r') as f:
        for line_number, line in enumerate(f, 1):
            try:
                parsed = parse_hex_line(line)
                results.append(parsed)
            except ValueError as e:
                print(f"Error on line {line_number}: {e}")
    return results


input_file = "BRAM_INIT.txt"  # Replace with your file
parsed_data = read_and_parse_file(input_file)

for i, (ints_32, int_8, int_12) in enumerate(parsed_data):
    print(f"Line {i+1}:")
    print(f"  32-bit ints: {ints_32}")
    print(f"  8-bit int : {int_8}")
    print(f"  12-bit int: {int_12}")
