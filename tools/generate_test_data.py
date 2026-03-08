# VeloDB: High-Performance Asynchronous Storage Engine
# Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
# 
# This software and the associated Nexus Protocol (NXP) are the 
# intellectual property of Amit Nilajkar.
# Licensed under the MIT License. See LICENSE file for details.

import sys
import os
import time

# Add the build directory to the path so we can import velodb_py
# Adjust this to match your build output location
build_dir = os.path.join(os.path.dirname(__file__), "..", "build", "Release")
sys.path.append(build_dir)

try:
    import velodb_py
except ImportError as e:
    print(f"Error: Could not import velodb_py. Ensure it is built in {build_dir}")
    print(e)
    sys.exit(1)

def main():
    if len(sys.argv) < 2:
        print("Usage: python generate_test_data.py <data_dir> [num_keys]")
        sys.exit(1)

    data_dir = sys.argv[1]
    num_keys = int(sys.argv[2]) if len(sys.argv) > 2 else 1_000_000

    print(f"Initializing VeloDB in {data_dir}...")
    db = velodb_py.DB(data_dir)

    print(f"Generating {num_keys} unique keys...")
    start_time = time.time()
    
    # Batch processing to show progress
    batch_size = 50_000
    for i in range(0, num_keys, batch_size):
        end = min(i + batch_size, num_keys)
        for k in range(i, end):
            # Using k as key and k*2 as value for easy verification
            db.put(k, k * 2)
        
        elapsed = time.time() - start_time
        throughput = (i + batch_size) / elapsed if elapsed > 0 else 0
        print(f"Progress: {end}/{num_keys} ({throughput:.2f} ops/sec)")

    total_time = time.time() - start_time
    print(f"\nSuccess! Injected {num_keys} keys in {total_time:.2f} seconds.")
    print(f"Average Throughput: {num_keys / total_time:.2f} ops/sec")

if __name__ == "__main__":
    main()

