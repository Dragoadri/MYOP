import time
import sys
import tty, termios

def getch():
  fd = sys.stdin.fileno()
  old_settings = termios.tcgetattr(fd)
  try:
    tty.setraw(sys.stdin.fileno())
    ch = sys.stdin.read(1)
  finally:
    termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
  return ch

def keyboard_poll_thread():
  import zmq
  context = zmq.Context()
  socket = context.socket(zmq.REQ) # block on send
  socket.bind('tcp://127.0.0.1:4444')

  while True:
    c = getch()
    print("got %s" % c)
    if c == '1':
      socket.send_string(str("cruise_up"))
      _ = socket.recv()
    if c == '2':
      socket.send_string(str("cruise_down"))
      _ = socket.recv()
    if c == '3':
      socket.send_string(str("cruise_cancel"))
      _ = socket.recv()
    if c == 'q':
      exit(0)

if __name__ == '__main__':
  keyboard_poll_thread()

