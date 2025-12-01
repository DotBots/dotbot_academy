import time
from pynput import keyboard

from marilib.mari_protocol import DefaultPayload, Frame
from marilib.marilib_cloud import MarilibCloud
from marilib.model import EdgeEvent, GatewayInfo, MariNode
from marilib.communication_adapter import MQTTAdapter

CMD_FORWARD = b'F'
CMD_LEFT = b'L'
CMD_RIGHT = b'R'
CMD_STOP = b'S'
DOTBOT_ADDRESS = 0xC81D8AF81A144EB4

def on_event(event: EdgeEvent, event_data: MariNode | Frame | GatewayInfo):
    return  # or print for debug

def main():
    mari = MarilibCloud(
        on_event,
        mqtt_interface=MQTTAdapter.from_url("mqtt://localhost:1883", is_edge=False),
        network_id=0xA000,
    )

    print("Connected. Use keys to control the robot (no Enter needed):")
    print("  w = forward, a = left, d = right, s = stop, l = quit")

    running = True

    def on_press(key):
        nonlocal running
        try:
            c = key.char.lower()
        except AttributeError:
            return

        if c == "w":
            payload = CMD_FORWARD
        elif c == "a":
            payload = CMD_LEFT
        elif c == "d":
            payload = CMD_RIGHT
        elif c == "s":
            payload = CMD_STOP
        elif c == "l":
            print("\nQuitting.")
            running = False
            return
        else:
            return

        mari.send_frame(DOTBOT_ADDRESS, payload)
        print(f" -> sent {payload!r} to {hex(DOTBOT_ADDRESS)}")

    listener = keyboard.Listener(on_press=on_press)
    listener.start()

    try:
        while running:
            mari.update()
            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\nInterrupted, exiting…")
    finally:
        listener.stop()
        listener.join()

if __name__ == "__main__":
    main()