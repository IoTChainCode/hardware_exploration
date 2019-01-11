# Project Name : blockchain/BlockchainMfc
# 
# These are used for demostration of ITC technical solution.
# It is also used as a device C-Model for firmware porting on chip, verify the functions and help to debug the firmware code.
# 
# Function Steps:
# 1. read initial parameter file initial.ini
# 2. create n threads simulate devices(1 thread stands for 1 device)
# 3. create 1 thread simulate mainchain
# 4. connect flow among n device threads
# 5. transaction flow between device threads and mainchain thread
# 6. ledger flow to confirm devices' and mainchain's ledger
# 
# Folders:
# blockchain : Code project of ITC solution, use command interface
# BlockchainMfc : Code project of ITC solution, use gui(mfc) interface
#
# Note : code for mainchain function in this C-Model is just a test framework, not directly derived from ITC mainchain ts.


# Project Name : app
# 
# This is used for merge ITC code into Android devices such as cellphone/pad.
# As a verify tool, this app can establish connection and transfer data with IoT devices by BLE interface.
# It also can be used as a full node, building a communication with ITC main network by 3G/4G interface, to make the transaction transfer originated from light/full node possible. 
# This code should be used combine with hardware_project.git.
# This full node is about MByte/GByte size both in code and data memory.


# Project Name : cc2540/cc2650/stm32l431
# 
# This is used for merging ITC code into IoT devices.
# 
# Project cc2540
# According to code from hardware_model.git, we use TI CC2540 BLE development kit as an embedded IoT chip platform to perform the project.
# This light node is about KByte size both in code and data memory, using BLE interface to communicate with full node.
# 
# Project cc2650
# In order to establish the BLE Mesh network, we choose the TI CC2650 chip.
# Differed with CC2540, this type of BLE chip can act as master/slave in parallel, which make the mesh network possible.
# 
# Project stm32l431
# Evb board combine with ST Microelectronics' stm32l431, Quectel's NB-IoT module BC35-G and multiple sensors, which can pack the temperature, humidity, gps data into specific format.
# Data is filtered by hash algorithm, encrypted by ECDSA signature algorithm, then send out to the ITC main-net by 4G MONET.