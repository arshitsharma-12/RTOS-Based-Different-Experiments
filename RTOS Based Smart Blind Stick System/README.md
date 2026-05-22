This project is a Smart Blind Stick Simulator developed using the STM32 Nucleo-F446RE and FreeRTOS. The system is designed to detect nearby obstacles in real time using an HC-SR04 ultrasonic sensor and provide adaptive alerts through a buzzer and LED.

The ultrasonic sensor continuously measures the distance of surrounding objects. Based on the detected distance, the system dynamically changes the buzzer beep frequency and LED blinking speed. As the obstacle gets closer, the alert speed increases, helping improve user awareness and reaction time.

The project demonstrates important embedded systems and RTOS concepts such as multitasking, task prioritization, message queues, and inter-task communication. Separate FreeRTOS tasks are used for distance sensing, alert decision-making, and output control, making the system modular and efficient.

This project represents a low-cost assistive technology solution and can be further extended with features like vibration feedback, voice guidance, or GPS-based navigation.
