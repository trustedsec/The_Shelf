import hashlib
import sys

def sha256_hash_string(input_string):
    # Calculate SHA256 hash of the input string
    hash_object = hashlib.sha256(input_string.encode('utf8'))
    hash_hex = hash_object.hexdigest()

    # Convert hexdigest to the desired format
    formatted_hash = ''.join('\\x' + hash_hex[i:i+2] for i in range(0, len(hash_hex), 2))
    return formatted_hash

# Example usage
input_string = sys.argv[1]
hashed_string = sha256_hash_string(input_string)
print(hashed_string)

