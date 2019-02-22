import struct
import sys
import serial

serData = serial.Serial('/dev/ttyACM1', 921600)

#
# TODO 1: (NOW FIXED) Find the first occurrence of magic and start from there
# TODO 2: Warn if we cannot parse a specific section and try to recover
# TODO 3: Remove error at end of file if we have only fragment of TLV
#

def tlvHeaderDecode(data):
    tlvType, tlvLength = struct.unpack('2I', data)
    return tlvType, tlvLength

def parsePointCloud(data, tlvLength):
    numPoints = tlvLength / 16
    for i in range(numPoints):
        print("\tPoint # : %2d/%2d" % (i+1, numPoints))
        Range, Azimuth, Doppler, Snr = struct.unpack('4f', data[16*i:16*i+16])
        print("\t\tRng :\t%07.3f" % (Range))
        print("\t\tAzm :\t%07.3f" % (Azimuth))
        print("\t\tDpl :\t%07.3f" % (Doppler))
        print("\t\tSnr :\t%07.3f" % (Snr))

def parseTargetList(data, tlvLength):
    numTarget = tlvLength / 68
    for i in range(numTarget):
        print("\tTarget # : %2d/%2d" % (i+1, numTarget))
        Tid, posX, posY, velX, velY, accX, accY, ec1, ec2, ec3, ec4, ec5, ec6, ec7, ec8, ec9, Gain = struct.unpack('I16f', data[68*i:68*i+68])
        print("\t\tT ID : %d" % (Tid))
        print("\t\tposX : %07.3f" % (posX))
        print("\t\tposY : %07.3f" % (posY))
        print("\t\tvelX : %07.3f" % (velX))
        print("\t\tvelY : %07.3f" % (velY))
        print("\t\taccX : %07.3f" % (accX))
        print("\t\taccY : %07.3f" % (accY))
        print("\t\tGain : %07.3f" % (Gain))

def parseTargetIndex(data, tlvLength):
    numIndex = tlvLength;
    for i in range(numIndex):
        print("\tIndex # : %2d/%2d" % (i+1, numIndex))
        targetId = struct.unpack('B', data[i])
        print("\t\tTarget ID :\t%d " % (targetId))

def tlvHeader(data):
    while data:
        headerLength = 52
        try:
            Magic, Version, Platform, timestamp, packetLength, frameNum, subframenum, chirpMargin, frameMargin, uartSentTime, trackProcTime, numTLVs, checkSum = struct.unpack('Q10I2H', data[:headerLength])
        except:
            print(">> Improper TLV structure found !!!\n")
            break
        print(">>         Magic : %x" % (Magic))
        print(">>       Version : %x" % (Version))
        print(">>      Platform : %x" % (Platform))
        print(">>  PacketLength : %x (%d)" % (packetLength, packetLength))
        print(">>      FrameNum : %x (%d)" % (frameNum, frameNum))
        print(">>   ChirpMargin : %x" % (chirpMargin))
        print(">>   FrameMargin : %x" % (frameMargin))
        print(">>      UartTime : %x" % (uartSentTime))
        print(">> TrackProcTime : %x" % (trackProcTime))
        print(">>       numTLVs : %x (%d)" % (numTLVs, numTLVs))
        print(">>      CheckSum : %x (%d)" % (checkSum, checkSum))
        paddingBytes = packetLength - headerLength
        data = data[headerLength:]
        for i in range(numTLVs):
            tlvType, tlvLength = tlvHeaderDecode(data[:8])
            print("...[%2d/%2d]" % (i+1, numTLVs))
            print("  >>   tlvType :\t%x (%d)" % (tlvType, tlvType))
            print("  >> tlvLength :\t%x (%d)" % (tlvLength, tlvLength))
            data = data[8:]
            length = tlvLength - 8
            if (tlvType == 6):
                print("  >> [%d] Point Cloud" % (tlvType))
                parsePointCloud(data, length)
            elif (tlvType == 7):
                print("  >> [%d] Target List" % (tlvType))
                parseTargetList(data, length)
            elif (tlvType == 8):
                print("  >> [%d] Target Index" % (tlvType))
                parseTargetIndex(data, length)
            else:
                print("  >> Unidentified TLV Type: %d" % (tlvType))
            data = data[length:]
            paddingBytes -= length
        print(">> PaddingBytes:\t%x (%d)" % (paddingBytes, paddingBytes))
        #data = data[paddingBytes:]
        yield packetLength, frameNum

if __name__ == "__main__":
    magic = b'\x02\x01\x04\x03\x06\x05\x08\x07'
    ping = serData.read(1500)
    offset = ping.find(magic)
    ping = ping[offset:]
    print("\n>> Start -> magic found (offset: %d)\n" % (offset))
    Magic = struct.unpack('Q', ping[:8])
    print(">> Magic %x\n" % (Magic))
    while 1:
        buff = ping[8:]
        offset = buff.find(magic)
        print(" [ 1 ] offset : %d\n" % (offset))
        if (offset > 8):
            packet = ping[:offset+8]
            for packetLength, frameNum in tlvHeader(packet):
                f_packet = 0
                print
            ping = ping[offset+8:]
            offset = 0
        else:
            pong = serData.read(1500)
            ping = ping + pong
            buff = ping[8:]
            offset = buff.find(magic)
            print(" [ 2 ] offset : %d\n" % (offset))
            packet = ping[:offset+8]
            for packetLength, frameNum in tlvHeader(packet):
                f_packet = 0
                print
            ping = ping[offset+8:]
            offset = 0

    serData.close()
