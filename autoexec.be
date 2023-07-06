def getCurrentTime()
 var timeMap = tasmota.rtc()
 var timeString = tasmota.strftime("%H: %M: %S", timeMap['local'])
 tasmota.resp_cmnd(timeString)
end

tasmota.add_cmd("TimeNow", getCurrentTime)

#gpio.I2C_SCL, gpio.I2C_SDA

ser = serial(-1,gpio.I2C_SDA, 115200,serial.SERIAL_8N1)
