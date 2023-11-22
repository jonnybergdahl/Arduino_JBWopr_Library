import paho.mqtt.client as mqtt


class MqttTestClient:
    def __init__(self):
        # MQTT broker IP address
        self.device_name = "wopr-461da0d8"
        self.mqtt_broker = "172.30.2.64"
        self.mqtt_username = "mqttclient"
        self.mqtt_password = "vG4pbft@QCir"
        self.mqtt_port = 1883
        self.mqtt_client = mqtt.Client()
        self.mqtt_client.on_connect = self.on_connect

    # Function to handle connection to the MQTT broker
    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT broker")
            client.subscribe("wopr/" + self.device_name + "/#")
        else:
            print("Connection to MQTT broker failed")

    def on_message(self, client, userdata, msg):
        print("< " + msg.topic + " " + str(msg.payload))

    def get_topic(self, command):
        if command == "DisplayState":
            return "wopr/" + self.device_name + "/display/state/set"
        elif command == "DisplayText":
            return "wopr/" + self.device_name + "/display/text/set"
        elif command == "DisplayScrollText":
            return "wopr/" + self.device_name + "/display/scrolltext/set"
        elif command == "DisplayBrightness":
            return "wopr/" + self.device_name + "/display/brightness/set"
        return ""

    def mqtt_connect(self):
        # Connect to the MQTT broker
        self.mqtt_client.connect(self.mqtt_broker, self.mqtt_port)
        self.mqtt_client.on_message = self.on_message
        self.mqtt_client.loop_start()

    def publish_message(self, topic, payload):
        if not self.mqtt_client.is_connected():
            self.mqtt_connect()
        print("> " + topic + " " + str(payload))
        info = self.mqtt_client.publish(topic, payload, qos=1)
        print(info.rc)

    def execute(self):
        # Create an MQTT client
        self.mqtt_client.username_pw_set(
            username=self.mqtt_username, password=self.mqtt_password
        )

        self.mqtt_connect()

        try:
            while True:
                # Prompt user for command and data
                command = input(
                    "Enter command (DisplayState, DisplayText, DisplayBrightness, DefconLevel): "
                ).strip()
                data = input("Enter data: ").strip()

                publish_topic = self.get_topic(command)
                if publish_topic == "":
                    print("Invalid command")
                    continue
                self.publish_message(publish_topic, data)

        except KeyboardInterrupt:
            # Disconnect from the MQTT broker when the script is interrupted
            self.mqtt_client.disconnect()


if __name__ == "__main__":
    client = MqttTestClient()
    client.execute()
