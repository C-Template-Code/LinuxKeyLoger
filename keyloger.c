#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <libudev.h>
#include <linux/input.h>

#define KEYBOARD_SUBSYSTEM "input"
#define KEYBOARD_USAGE_PAGE 1
#define KEYBOARD_USAGE 6

int main() {
    
    struct udev *udev;
    struct udev_monitor *mon;
    int fd;

    // Создаем контекст udev
    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Не удалось создать контекст udev\n");
        exit(EXIT_FAILURE);
    }

    // Создаем монитор udev для событий ввода
    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, KEYBOARD_SUBSYSTEM, NULL);
    udev_monitor_enable_receiving(mon);

    // Получаем файловый дескриптор монитора
    fd = udev_monitor_get_fd(mon);

    printf("Нажмите Ctrl+C для выхода.\n");

    // Открываем устройство клавиатуры
    int keyboard_fd = open("/dev/input/event3", O_RDONLY);
    if (keyboard_fd < 0) {
        perror("Не удалось открыть устройство клавиатуры");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Ожидаем события от устройства клавиатуры
        struct input_event ev; 
        if (read(keyboard_fd, &ev, sizeof(ev)) != sizeof(ev)) {
            perror("Ошибка чтения с устройства клавиатуры");
            exit(EXIT_FAILURE);
        }

        // Проверяем, что это событие нажатия клавиши
        if (ev.type == EV_KEY && ev.value == 1) {
            printf("Клавиша с кодом %d нажата.\n", ev.code);
        }

        // Ожидаем события от udev монитора
        if (udev_monitor_receive_device(mon)) {
            struct udev_device *dev = udev_monitor_receive_device(mon);

            // Проверяем, что это устройство клавиатуры
            if (dev && udev_device_get_property_value(dev, "ID_INPUT_KEYBOARD")) {
                printf("Нажата клавиша на клавиатуре (через udev).\n");
            }

            udev_device_unref(dev);
        }
    }

    // Закрываем устройство клавиатуры
    close(keyboard_fd);

    // Освобождаем ресурсы
    udev_monitor_unref(mon);
    udev_unref(udev);

    return 0;
}
