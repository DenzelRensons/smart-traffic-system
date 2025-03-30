#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Traffic Sensor structure
typedef struct {
    int sensor_id;
    char location[50];
    int current_status; // 0=red, 1=yellow, 2=green
    time_t last_updated;
    float* sensor_readings;
    size_t readings_count;
    size_t readings_capacity;
    bool is_active;
} TrafficSensor;

// Global system variables
TrafficSensor** sensors = NULL;
size_t sensor_count = 0;
size_t sensor_capacity = 0;
const char* status_names[] = {"RED", "YELLOW", "GREEN"};

// Function prototypes
TrafficSensor* find_sensor(int id);
int add_sensor(int id, const char* loc);
int remove_sensor(int id);
int add_sensor_reading(int sensor_id, float value);
int update_sensor_status(int id, int status);
void garbage_collect();
void cleanup_system();
void display_all_sensors(bool show_inactive);
void handle_buffer_overflow(TrafficSensor* sensor);
int validate_sensor_data(float value);

int main() {
    printf("\nSmart City Traffic Light Management System\n");
    printf("========================================\n");

    int choice;
    while (1) {
        printf("\nMain Menu:\n");
        printf("1. Add Traffic Sensor\n");
        printf("2. Remove Traffic Sensor\n");
        printf("3. Add Sensor Reading\n");
        printf("4. Update Traffic Light Status\n");
        printf("5. Display All Active Sensors\n");
        printf("6. Display All Sensors (including inactive)\n");
        printf("7. Run Garbage Collection\n");
        printf("8. Exit System\n");
        printf("Enter your choice (1-8): ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        switch (choice) {
            case 1: {
                int id;
                char loc[50];
                printf("Enter sensor ID: ");
                scanf("%d", &id);
                printf("Enter location: ");
                scanf("%49s", loc);
                if (add_sensor(id, loc) != 0) {
                    printf("Failed to add sensor!\n");
                }
                break;
            }
            case 2: {
                int id;
                printf("Enter sensor ID to remove: ");
                scanf("%d", &id);
                if (remove_sensor(id) != 0) {
                    printf("Failed to remove sensor!\n");
                }
                break;
            }
            case 3: {
                int id;
                float value;
                printf("Enter sensor ID: ");
                scanf("%d", &id);
                printf("Enter reading value: ");
                scanf("%f", &value);
                if (add_sensor_reading(id, value) != 0) {
                    printf("Failed to add reading!\n");
                }
                break;
            }
            case 4: {
                int id, status;
                printf("Enter sensor ID: ");
                scanf("%d", &id);
                printf("Enter new status (0=RED, 1=YELLOW, 2=GREEN): ");
                scanf("%d", &status);
                if (update_sensor_status(id, status) != 0) {
                    printf("Failed to update status!\n");
                }
                break;
            }
            case 5:
                display_all_sensors(false);
                break;
            case 6:
                display_all_sensors(true);
                break;
            case 7:
                garbage_collect();
                break;
            case 8:
                cleanup_system();
                printf("System shutdown. All memory freed.\n");
                return 0;
            default:
                printf("Invalid choice! Please enter 1-8.\n");
        }
    }
}

// [Implementation of all functions...]

TrafficSensor* find_sensor(int id) {
    for (size_t i = 0; i < sensor_count; i++) {
        if (sensors[i] && sensors[i]->sensor_id == id) {
            return sensors[i];
        }
    }
    return NULL;
}

int add_sensor(int id, const char* loc) {
    if (find_sensor(id)) {
        printf("Error: Sensor ID %d already exists!\n", id);
        return -1;
    }

    // Resize sensors array if needed (dynamic allocation)
    if (sensor_count >= sensor_capacity) {
        size_t new_capacity = sensor_capacity ? sensor_capacity * 2 : 4;
        TrafficSensor** new_sensors = realloc(sensors, new_capacity * sizeof(TrafficSensor*));
        if (!new_sensors) {
            printf("Memory allocation failed for sensors array!\n");
            return -1;
        }
        sensors = new_sensors;
        sensor_capacity = new_capacity;
    }

    // Allocate memory for new sensor
    TrafficSensor* new_sensor = malloc(sizeof(TrafficSensor));
    if (!new_sensor) {
        printf("Memory allocation failed for new sensor!\n");
        return -1;
    }

    // Initialize sensor data
    new_sensor->sensor_id = id;
    strncpy(new_sensor->location, loc, sizeof(new_sensor->location) - 1);
    new_sensor->location[sizeof(new_sensor->location) - 1] = '\0';
    new_sensor->current_status = 0; // Default to RED
    new_sensor->last_updated = time(NULL);
    new_sensor->is_active = true;
    new_sensor->readings_count = 0;
    new_sensor->readings_capacity = 4; // Initial capacity
    
    // Allocate memory for readings (dynamic allocation)
    new_sensor->sensor_readings = calloc(new_sensor->readings_capacity, sizeof(float));
    if (!new_sensor->sensor_readings) {
        printf("Memory allocation failed for sensor readings!\n");
        free(new_sensor);
        return -1;
    }

    sensors[sensor_count++] = new_sensor;
    printf("Sensor %d at %s added successfully.\n", id, loc);
    return 0;
}

int remove_sensor(int id) {
    for (size_t i = 0; i < sensor_count; i++) {
        if (sensors[i] && sensors[i]->sensor_id == id) {
            // Free all allocated memory for this sensor
            free(sensors[i]->sensor_readings);
            free(sensors[i]);
            
            // Shift remaining sensors
            for (size_t j = i; j < sensor_count - 1; j++) {
                sensors[j] = sensors[j + 1];
            }
            
            sensor_count--;
            printf("Sensor %d removed successfully.\n", id);
            return 0;
        }
    }
    printf("Error: Sensor %d not found!\n", id);
    return -1;
}

int add_sensor_reading(int sensor_id, float value) {
    TrafficSensor* sensor = find_sensor(sensor_id);
    if (!sensor) {
        printf("Error: Sensor %d not found!\n", sensor_id);
        return -1;
    }

    // Validate sensor data (buffer overflow protection)
    if (validate_sensor_data(value) != 0) {
        handle_buffer_overflow(sensor);
        return -1;
    }

    // Resize readings array if needed (dynamic reallocation)
    if (sensor->readings_count >= sensor->readings_capacity) {
        size_t new_capacity = sensor->readings_capacity * 2;
        float* new_readings = realloc(sensor->sensor_readings, new_capacity * sizeof(float));
        if (!new_readings) {
            printf("Memory reallocation failed for sensor readings!\n");
            return -1;
        }
        sensor->sensor_readings = new_readings;
        sensor->readings_capacity = new_capacity;
    }

    sensor->sensor_readings[sensor->readings_count++] = value;
    sensor->last_updated = time(NULL);
    printf("Reading %.2f added to sensor %d.\n", value, sensor_id);
    return 0;
}

int update_sensor_status(int sensor_id, int status) {
    if (status < 0 || status > 2) {
        printf("Error: Invalid status code!\n");
        return -1;
    }

    TrafficSensor* sensor = find_sensor(sensor_id);
    if (!sensor) {
        printf("Error: Sensor %d not found!\n", sensor_id);
        return -1;
    }

    sensor->current_status = status;
    sensor->last_updated = time(NULL);
    printf("Sensor %d status changed to %s.\n", sensor_id, status_names[status]);
    return 0;
}

void garbage_collect() {
    time_t now = time(NULL);
    const time_t STALE_THRESHOLD = 3600; // 1 hour in seconds
    int collected = 0;

    for (size_t i = 0; i < sensor_count; ) {
        if (difftime(now, sensors[i]->last_updated) > STALE_THRESHOLD) {
            printf("Marking sensor %d as inactive (last updated: %ld seconds ago)\n", 
                   sensors[i]->sensor_id, (long)difftime(now, sensors[i]->last_updated));
            sensors[i]->is_active = false;
            collected++;
            i++;
        } else {
            i++;
        }
    }

    printf("Garbage collection completed. %d sensors marked inactive.\n", collected);
}

void cleanup_system() {
    for (size_t i = 0; i < sensor_count; i++) {
        if (sensors[i]) {
            free(sensors[i]->sensor_readings);
            free(sensors[i]);
        }
    }
    free(sensors);
    sensor_count = 0;
    sensor_capacity = 0;
}

void display_all_sensors(bool show_inactive) {
    printf("\n=== Sensor List ===\n");
    if (sensor_count == 0) {
        printf("No sensors in system.\n");
        return;
    }

    int displayed = 0;
    for (size_t i = 0; i < sensor_count; i++) {
        if (show_inactive || sensors[i]->is_active) {
            printf("\nSensor ID: %d\n", sensors[i]->sensor_id);
            printf("Location: %s\n", sensors[i]->location);
            printf("Status: %s\n", status_names[sensors[i]->current_status]);
            printf("Last Updated: %ld seconds ago\n", 
                  (long)difftime(time(NULL), sensors[i]->last_updated));
            printf("Active: %s\n", sensors[i]->is_active ? "YES" : "NO");
            printf("Readings (%zu/%zu): ", sensors[i]->readings_count, sensors[i]->readings_capacity);
            
            for (size_t j = 0; j < sensors[i]->readings_count; j++) {
                printf("%.2f ", sensors[i]->sensor_readings[j]);
            }
            printf("\n");
            displayed++;
        }
    }

    if (!displayed) {
        printf("No sensors match the display criteria.\n");
    } else {
        printf("\nTotal displayed: %d\n", displayed);
    }
}

int validate_sensor_data(float value) {
    // Check for corrupt data (buffer overflow protection)
    if (value < 0 || value > 1000) { // Example validation range
        printf("Warning: Corrupt sensor data detected (%.2f)\n", value);
        return -1;
    }
    return 0;
}

void handle_buffer_overflow(TrafficSensor* sensor) {
    printf("Handling potential buffer overflow for sensor %d\n", sensor->sensor_id);
    // Reset readings to prevent corruption
    sensor->readings_count = 0;
    memset(sensor->sensor_readings, 0, sensor->readings_capacity * sizeof(float));
    sensor->last_updated = time(NULL);
}
