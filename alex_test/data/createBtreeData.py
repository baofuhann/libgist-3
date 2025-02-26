#!/usr/bin/python
# -*- coding: utf-8 -*-

import random

keys = sorted(random.sample(range(1, 501), 500))

with open("btreedata.txt", "w") as fout:
    fout.write("500\n")  

    for key in keys:
        data = random.randint(100, 201)  
        fout.write(f"{key}\t{data}\n")  
