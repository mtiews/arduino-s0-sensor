s0-Sensor including SmartHome.py plug-in
----------------------------------------

Small project to read s0 impulses of power meters and make them available in SmartHome.py. 

### Description

* (Arduino-compatible) Nano V3.0 based circuit
* 4 s0 interfaces supported
* including temperature/humidity sensor (DHT22)
* USB connection to host (currently Raspberry Pi running SmartHome.pi)
* SmartHome.py plug-in to receive values submitted by this sensor

### Smarthome.py plug-in

Item configuration for sensor:
```
[sensors]
    [[epower_house]]
        type=num
        sensors_var=cceed198-81c9-4f62-9592-374c5fc8611c/W
        visu_acl=rw
        sqlite=yes
    [[ework_house_today]]
        # Will be set to -1 each night by a seperate smarthome.py script
        type=num
        sensors_var=cceed198-81c9-4f62-9592-374c5fc8611c/WH
        visu_acl=rw
        eval = 0 if (int(value) < 0) else (sh.sensors.ework_house_today() + (int(value)/1000))
        sqlite=yes
    [[ework_house_yesterday]]
        type=num
        visu_acl=rw
    [[epower_heating]]
        type=num
        sensors_var=18c0055f-68d3-4749-b335-8b029cf1cf16/W
        visu_acl=rw
        sqlite=yes
    [[ework_heating_today]]
        # Will be set to -1 each night by a seperate smarthome.py script
        type=num
        sensors_var=18c0055f-68d3-4749-b335-8b029cf1cf16/WH
        visu_acl=rw
        eval = 0 if (int(value) < 0) else (sh.sensors.ework_heating_today() + (int(value)/1000))
        sqlite=yes
    [[ework_heating_yesterday]]
        type=num
        visu_acl=rw
    [[temp_hum]]
        [[[temperature]]]
            type=num
            sensors_var=8eff16c9-b53b-4d08-8051-ee3737fd0332/TEMP
            visu_acl=rw
        [[[humidity]]]
            type=num
            sensors_var=8eff16c9-b53b-4d08-8051-ee3737fd0332/HUM
            visu_acl=rw
```

For other use cases (e.g. only count the impulses) you can use (for all 4 S0 inputs) the `sensor_var` named `<id>/Impulses`.

Sensor plug-in configuration in plugin.conf:

```
[sensors]
    class_name = Sensors
    class_path = plugins.sensors
    tty = /dev/arduino_s0
```
