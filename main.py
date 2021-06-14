#!/usr/bin/python3
import RPi.GPIO as GPIO
import time
import rospy
from geometry_msgs.msg import Twist
from std_msgs.msg import Float64
from sensor_msgs.msg import Joy
# IC2 bus 
from smbus2 import SMBus
# Nvidia Jetson Nano I2C Bus 0
bus = SMBus(1)
# IC2 bus address
address2 = 0x50
address1 = 0x60


def writeNumber(value):
    #bus.write_byte(address, int(value))
    # bus.write_byte_data(address, 0, value)
    return -1

# Writes an integer array to specific address
# This is a mess rn needs to be optmized
def writeBlock(joyVal, deviceId, address):
    n = joyVal
    digit = 0
    #while(n > 0):
        #digit += 1
        #n /= 10

    data = [0, 0, 0]
    dataDup = [0, 0, 0]

    index = 0

    while(index < 3):
       dataDup[index] = int(n%10)
       index += 1
       n = n // 10

    for i in range(0, index):
        data[i] = dataDup[index-i-1]
    # Writes the integer array to the address    
    bus.write_i2c_block_data(address, deviceId, data)
    print(data)

def readNumber():
    number = bus.read_byte(address)
    # number = bus.read_byte_data(address, 1)
    return number

def callback(data):
    # Translates the axis data into a range depending on function
    # Functions for tank drive and belt
    leftSide = translate(data.axes[4], -1.0, 1.0, 0, 180)
    rightSide = translate(data.axes[1], -1.0, 1.0, 0, 180)
    belt = translate(data.axes[5], -1.0, 1.0, 180, 90)
    # Writes values
    writeBlock(leftSide, 1, address2)
    writeBlock(rightSide, 2, address2)
    writeBlock(belt, 3, address2)

    # rotate away from digging pos X button
    # device ID 1 
    if data.buttons[2] == 1:
        writeBlock(120, 1, address1)
    # rotate towards digging pos B button
    if data.buttons[1] == 1:
        writeBlock(40, 1, address1)
    # stop rotation if nothing is pressed
    if data.buttons[1] == 0 and data.buttons[2] == 0:
        writeBlock(90, 1, address1)
    # translate towards ground A button
    if data.buttons[0] == 1:
        writeBlock(100, 2, address1)
    # translate away ground B button
    if data.buttons[3] == 1:
        writeBlock(200, 2, address1)
    # stop translation if nothing is pressed
    if data.buttons[3] == 0 and data.buttons[0] == 0:
        writeBlock(150, 2, address1)
    # left trigger controls dig speed
    digBelt = translate(data.axes[2], -1.0, 1.0, 180, 90)
    writeBlock(digBelt, 3, address1)
    
    print(round(leftSide))
    print(round(rightSide))
    print(round(belt))

def main():
    rospy.init_node('nasa_control')
    sub = rospy.Subscriber('joy', Joy, callback)
    rate = rospy.Rate(100)
    rospy.spin()


def translate(inp, leftMin, leftMax, rightMin, rightMax):
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin
    valueScaled = float(inp - leftMin)/ float(leftSpan)
    return rightMin + (valueScaled * rightSpan)



if __name__ =='__main__':
    main()
