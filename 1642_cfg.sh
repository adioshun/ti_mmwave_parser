echo "sensorStop" > /dev/ttyACM0
sleep 0.3
echo "sensorStop" > /dev/ttyACM0
sleep 0.3
echo "flushCfg" > /dev/ttyACM0
sleep 0.3
echo "dfeDataOutputMode 1" > /dev/ttyACM0
sleep 0.3
echo "channelCfg 15 3 0" > /dev/ttyACM0
sleep 0.3
echo "adcCfg 2 1" > /dev/ttyACM0
sleep 0.3
echo "adcbufCfg 0 1 1 1" > /dev/ttyACM0
sleep 0.3
echo "profileCfg 0 77 30 7 62 0 0 60 1 128 2500 0 0 30" > /dev/ttyACM0
sleep 0.3
echo "chirpCfg 0 0 0 0 0 0 0 1" > /dev/ttyACM0
sleep 0.3
echo "chirpCfg 1 1 0 0 0 0 0 2" > /dev/ttyACM0
sleep 0.3
echo "frameCfg 0 1 128 0 50 1 0" > /dev/ttyACM0
sleep 0.3
echo "lowPower 0 1" > /dev/ttyACM0
sleep 0.3
echo "guiMonitor 1 1 0 0" > /dev/ttyACM0
sleep 0.3
echo "cfarCfg 6 4 4 0 0 16 16 4 4 50 62 0" > /dev/ttyACM0
sleep 0.3
echo "doaCfg 600 1875 30 1" > /dev/ttyACM0
sleep 0.3
echo "SceneryParam -6 6 0.05 6" > /dev/ttyACM0
sleep 0.3
echo "GatingParam 4 3 2 0" > /dev/ttyACM0
sleep 0.3
echo "StateParam 10 5 10 100 5" > /dev/ttyACM0
sleep 0.3
echo "AllocationParam 450 0.01 25 1 2" > /dev/ttyACM0
sleep 0.3
echo "VariationParam 0.289 0.289 1.0" > /dev/ttyACM0
sleep 0.3
echo "PointCloudEn 0" > /dev/ttyACM0
sleep 0.3
echo "trackingCfg 1 2 250 20 200 50 90" > /dev/ttyACM0
sleep 0.3
echo "sensorStart" > /dev/ttyACM0
