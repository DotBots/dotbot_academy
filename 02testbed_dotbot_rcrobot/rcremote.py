import time
from pynput import keyboard

from marilib.mari_protocol import Frame
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
    print("  w = forward, a = left, d = right, s = stop, q = quit")

    running = True
    pressed = set()          # currently pressed movement keys
    last_sent = {}           # key -> last send time (seconds)
    send_interval = 0.1      # seconds between frames for a held key

    def on_press(key):
        nonlocal running, pressed
        try:
            c = key.char.lower()
        except AttributeError:
            return

        if c in ("w", "a", "d", "s"):
            pressed.add(c)
        elif c == "q":
            print("\nQuitting.")
            running = False

    def on_release(key):
        nonlocal pressed
        try:
            c = key.char.lower()
        except AttributeError:
            return
        if c in pressed:
            pressed.discard(c)

    listener = keyboard.Listener(on_press=on_press, on_release=on_release)
    listener.start()

    try:
        while running:
            mari.update()

            now = time.time()
            for c in list(pressed):
                if c == "w":
                    payload = CMD_FORWARD
                elif c == "a":
                    payload = CMD_LEFT
                elif c == "d":
                    payload = CMD_RIGHT
                elif c == "s":
                    payload = CMD_STOP
                else:
                    continue

                last = last_sent.get(c, 0.0)
                if now - last >= send_interval:
                    mari.send_frame(DOTBOT_ADDRESS, payload)
                    last_sent[c] = now
                    print(f" -> sent {payload!r} to {hex(DOTBOT_ADDRESS)}")

            time.sleep(0.02)
    except KeyboardInterrupt:
        print("\nInterrupted, exiting…")
    finally:
        listener.stop()
        listener.join()

if __name__ == "__main__":
    main()