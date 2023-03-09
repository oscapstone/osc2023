import os
import pty
import subprocess

def screen_process():
    # Open a new pseudo-terminal
    master, slave = pty.openpty()
    # Associate the slave terminal with the serial device
    os.system('sudo ln -s /dev/tty.usbserial-0001 /dev/ttyUSB0')
    # Start the screen process and associate it with the pseudo-terminal
    p = subprocess.Popen(['screen', '/dev/ttyUSB0', '115200'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, start_new_session=True, preexec_fn=os.setsid, close_fds=True, cwd=os.getcwd(), pass_fds=(master,))
    # Forward input/output from the screen process to the master terminal
    while True:
        output = os.read(master, 1024)
        if not output:
            break
        os.write(1, output)
        os.write(slave, output)

if __name__ == '__main__':
    screen_process()
