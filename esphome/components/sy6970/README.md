# Overview
This component supports the sy6970 BMS chip.

[SY6970 Datasheet.pdf](https://github.com/Xinyuan-LilyGO/LilyGo-AMOLED-Series/blob/master/datasheet/SY6970%20Datasheet.pdf)

It is known to be used in the Lilygo [t-displayS3-pro](https://lilygo.cc/products/t-display-s3-pro).

## Working configuration for the t-displayS3-pro (v1)

```yaml
i2c:
  - id: bus_a
    scan: True
    sda: GPIO5
    scl: GPIO6

sy6970:
  id: pmu
  address: 0x6A

sensor:
  - platform: sy6970
    sy6970_id: pmu
    update_interval: 1s
    vbus_voltage:
      name: "USB Voltage"
    battery_voltage:
      name: "Battery Voltage"
    charge_current:
      name: "Charge Current"

text_sensor:
  - platform: sy6970
    sy6970_id: pmu
    update_interval: 1s
    bus_status:
      name: "Power Source Type"
      # Reports: "No Input", "USB SDP", "USB CDP", "USB DCP",
      #          "HVDCP", "Adapter", "Non-Standard Adapter", "OTG"
    charge_status:
      name: "Charging Status"
      # Reports: "Not Charging", "Pre-charge", "Fast Charge", "Charge Done"
    ntc_status:
      name: "Battery Temperature"
      # Reports: "Normal", "Warm", "Cool", "Cold", "Hot"

binary_sensor:
  - platform: sy6970
    sy6970_id: pmu
    update_interval: 1s
    charging:
      name: "Battery Charging"

```
