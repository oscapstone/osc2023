#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char ** argv) {
    if (argc < 2) {
        printf("usage: %s <tty-path> [kernel-path]\n", argv[0]);
        return -1;
    }

    char * tty_path = argv[1];
    char * kernel_path = argc < 3 ? "kernel8.img" : argv[2];

    int tty = open(tty_path, O_WRONLY | O_NOCTTY | O_NDELAY);
    if (tty == -1) {
        perror("open tty");
        return -1;
    }

    struct termios termios = {0};
    if (tcgetattr(tty, &termios) < 0) {
        perror("tcgetattr");
        return -1;
    }

    cfmakeraw(&termios);
    termios.c_cflag &= ~(CRTSCTS | CSTOPB);
    termios.c_cflag |= CLOCAL;

    if (cfsetispeed(&termios, B115200) < 0) {
        perror("cfsetispeed");
        return -1;
    }
    if (cfsetospeed(&termios, B115200) < 0) {
        perror("cfsetospeed");
        return -1;
    }

    if (tcflush(tty, TCIFLUSH) != 0) {
        perror("tcflush");
        return -1;
    }

    if (tcsetattr(tty, TCSANOW, &termios) < 0) {
        perror("tcsetattr");
        return -1;
    }

    struct stat st;
    if (stat(kernel_path, &st) == -1) {
        perror("stat");
        return -1;
    }

    off_t size = st.st_size;
    if (write(tty, &size, 4) != 4) {
        perror("write size");
        return -1;
    }
    usleep(1000000);

    int kfd = open(kernel_path, O_RDWR);
    if (kfd == -1) {
        perror("open kernel");
        return -1;
    }

    char * kernel = (char *)mmap(NULL, size, PROT_READ, MAP_PRIVATE, kfd, 0);
    if (kernel == MAP_FAILED) {
        perror("mmap kernel");
        return -1;
    }

    if (close(kfd) == -1) {
        perror("close kernel");
        return -1;
    }

    for (ssize_t w = 0, i = 0; i < size; i += w) {
        if ((w = write(tty, kernel + i, size - i)) < 0) {
            perror("write kernel");
            return -1;
        }
        printf("%zu\n", w);
        usleep(1000000);
    }

    if (munmap(kernel, size) != 0) {
        perror("munmap kernel");
        return -1;
    }

    if (close(tty) == -1) {
        perror("close tty");
        return -1;
    }
}
