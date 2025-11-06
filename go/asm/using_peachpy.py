#!/usr/bin/env python3

from peachpy.x86_64 import *
from peachpy import *

# Define arguments outside the function
x = Argument(uint64_t)
y = Argument(uint64_t)

with Function("Add", (x, y), uint64_t) as function:
    # Load arguments into registers using LOAD.ARGUMENT
    reg_x = GeneralPurposeRegister64()
    LOAD.ARGUMENT(reg_x, x)

    reg_y = GeneralPurposeRegister64()
    LOAD.ARGUMENT(reg_y, y)

    # Add them
    ADD(reg_x, reg_y)

    # Return the result (in reg_x)
    RETURN(reg_x)
