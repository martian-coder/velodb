# VeloDB: High-Performance Asynchronous Storage Engine
# Copyright (c) 2026 Amit Nilajkar <amit.nilajkar@gmail.com>
# 
# This software and the associated Nexus Protocol (NXP) are the 
# intellectual property of Amit Nilajkar.
# Licensed under the MIT License. See LICENSE file for details.

import sys
import os

# Add build directory to path to find the compiled module
build_dir = os.path.join(os.getcwd(), "build", "Release")
if not os.path.exists(build_dir):
    build_dir = os.path.join(os.getcwd(), "build")
sys.path.append(build_dir)

try:
    import velodb_py
    print("Successfully imported velodb_py")
except ImportError as e:
    print(f"Failed to import velodb_py: {e}")
    sys.exit(1)

def test_engine():
    db_path = "./testdata_py"
    if os.path.exists(db_path):
        import shutil
        shutil.rmtree(db_path)
    
    print(f"Initializing VeloDB at {db_path}...")
    db = velodb_py.DB(db_path)
    
    print("Testing PUT...")
    db.put(1, 100)
    db.put(2, 200)
    db.put(3, 300)
    
    print("Testing GET...")
    v1 = db.get(1)
    v2 = db.get(2)
    print(f"Get(1) = {v1}, Get(2) = {v2}")
    assert v1 == 100
    assert v2 == 200
    
    print("Testing RANGE...")
    res = db.range(1, 2)
    print(f"Range(1, 2) = {res}")
    assert len(res) == 2
    assert res[0] == (1, 100)
    assert res[1] == (2, 200)
    
    print("Python bindings test PASSED!")

if __name__ == "__main__":
    test_engine()

