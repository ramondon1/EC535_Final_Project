#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONTROL_FILE "/proc/av_control"
#define STATUS_FILE "/proc/av_sched_status"

void send_command(const char *cmd) {
    FILE *fp = fopen(CONTROL_FILE, "w");
    if (!fp) {
        perror("Failed to open control file");
        exit(EXIT_FAILURE);
    }

    fprintf(fp, "%s\n", cmd);
    fclose(fp);
}

void read_status() {
    char buffer[1024];
    FILE *fp = fopen(STATUS_FILE, "r");
    if (!fp) {
        perror("Failed to open status file");
        return;
    }

    printf("=== Current AV Scheduler Status ===\n");
    while (fgets(buffer, sizeof(buffer), fp)) {
        fputs(buffer, stdout);
    }

    fclose(fp);
}

void print_menu() {
    printf("\n=== AV Simulator ===\n");
    printf("1. Simulate object in path (trigger braking)\n");
    printf("2. Deactivate braking\n");
    printf("3. Simulate sensor data collection\n");
    printf("4. Simulate path planning\n");
    printf("5. Simulate vehicle control\n");
    printf("6. Simulate communication\n");
    printf("7. Simulate data logging\n");
    printf("8. Show scheduler status\n");
    printf("0. Exit\n");
    printf("Select an option: ");
}

int main() {
    int choice;

    while (1) {
        print_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                send_command("object 1");  // Simulate object detection, trigger braking
                break;
            case 2:
                send_command("brake 0");  // Deactivate braking
                break;
            case 3:
                send_command("sensor");  // Trigger sensor task manually
                break;
            case 4:
                send_command("path");  // Trigger path planning task manually
                break;
            case 5:
                send_command("control");  // Trigger vehicle control task manually
                break;
            case 6:
                send_command("comm");  // Trigger communication task manually
                break;
            case 7:
                send_command("log");  // Trigger data logging task manually
                break;
            case 8:
                read_status();  // Display the current status of tasks
                break;
            case 0:
                printf("Exiting simulator.\n");
                return 0;
            default:
                printf("Invalid option.\n");
                break;
        }
    }

    return 0;
}
