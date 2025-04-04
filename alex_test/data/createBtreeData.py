#!/usr/bin/python
# -*- coding: utf-8 -*-

import random

keys = sorted(random.sample(range(1, 500001), 50000))

with open("btreedata.txt", "w") as fout:
    fout.write("50000\n")  

    for key in keys:
        data = random.randint(1, 10000)
        fout.write(f"{key}\t{data}\n")  
