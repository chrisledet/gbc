import argparse

def read_byte_at(filename, position):
    with open(filename, 'rb') as file:
        file.seek(position)
        byte = file.read(1)
    return byte

def main():
    parser = argparse.ArgumentParser(description="Read a byte from a binary file at a specified position.")
    parser.add_argument("filename", help="The binary file name.")
    parser.add_argument("position", type=int, help="The byte position to read from.")
    args = parser.parse_args()

    byte_data = read_byte_at(args.filename, args.position)

    if byte_data:
        print(f"{args.position}: {byte_data} (Hex: {byte_data.hex()})")
    else:
        print(f"No data at position {args.position} or beyond the end of the file.")

if __name__ == "__main__":
    main()
