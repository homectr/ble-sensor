# nRF24 Sensor Node
Battery powered nRF24 sensor/actuator node. Collects data from enabled sensors and broadcasts them using nRF24.
Use nRF24-to-mqtt-bridge to receive such broadcasts and forward them to your mqtt. See https://github.com/homectr/nrf24-to-mqtt-bridge


## Supported sensors
1. temperature (celsius): DHT
1. relative humidity: DHT
1. contact - detects opening and closing of a contact
