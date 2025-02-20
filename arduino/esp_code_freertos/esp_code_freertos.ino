#include <WiFi.h>
#include <HTTPClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ESP32Servo.h>

// Pin definitions for parking sensors
#define IR_SENSOR_PIN1 19
#define IR_SENSOR_PIN2 18
#define IR_SENSOR_PIN3 5
#define IR_SENSOR_PIN4 17

// Pin definitions for the entry IR sensor and servo motor
#define ENTRY_SENSOR_PIN 4
#define SERVO_PIN 16

// Wi-Fi credentials
const char *ssid = "SOBRALNET";      // Wi-Fi SSID
const char *password = "1984511330"; // Wi-Fi password

// Server details
const char *serverUrl = "http://myipadress:3000/api/data"; // Server URL
const char *deviceId = "ESP32_01";                         // Unique ID for this ESP8266

// Mutex for shared resources
SemaphoreHandle_t xMutex = NULL;

// Sensor data structure
struct SensorData
{
    String space1;
    String space2;
    String space3;
    String space4;
};

SensorData sensorData;

// Create a Servo object
Servo myServo;

// Task handles
TaskHandle_t wifiTaskHandle = NULL;
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t httpTaskHandle = NULL;
TaskHandle_t servoTaskHandle = NULL;

// Function prototypes
void connectToWiFi();
void readSensors(void *parameter);
void sendDataToServer(void *parameter);
void monitorEntrySensor(void *parameter);

void setup()
{
    // Initialize serial communication
    Serial.begin(115200);

    // Set up parking sensor pins as input
    pinMode(IR_SENSOR_PIN1, INPUT);
    pinMode(IR_SENSOR_PIN2, INPUT);
    pinMode(IR_SENSOR_PIN3, INPUT);
    pinMode(IR_SENSOR_PIN4, INPUT);

    // Set up the entry sensor pin
    pinMode(ENTRY_SENSOR_PIN, INPUT);

    // Attach the servo motor to its control pin and set its initial position to 90°
    myServo.attach(SERVO_PIN);
    myServo.write(90);

    // Create a mutex for shared resources
    xMutex = xSemaphoreCreateMutex();
    if (xMutex == NULL)
    {
        Serial.println("Failed to create mutex");
        while (1)
            ; // Halt if mutex creation fails
    }

    // Connect to Wi-Fi
    connectToWiFi();

    // Create tasks
    xTaskCreatePinnedToCore(
        readSensors,       // Task function
        "SensorTask",      // Task name
        4096,              // Stack size
        NULL,              // Task parameters
        1,                 // Priority
        &sensorTaskHandle, // Task handle
        1                  // Core
    );

    xTaskCreatePinnedToCore(
        sendDataToServer, // Task function
        "HTTPTask",       // Task name
        8192,             // Stack size
        NULL,             // Task parameters
        2,                // Priority
        &httpTaskHandle,  // Task handle
        1                 // Core
    );

    xTaskCreatePinnedToCore(
        monitorEntrySensor, // Task function
        "ServoTask",        // Task name
        4096,               // Stack size
        NULL,               // Task parameters
        1,                  // Priority
        &servoTaskHandle,   // Task handle
        1                   // Core
    );
}

void loop()
{
    // FreeRTOS scheduler will handle task execution
    vTaskDelete(NULL);
}

// Task to read parking sensor data
void readSensors(void *parameter)
{
    while (1)
    {
        // Take the mutex to access shared sensor data
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            sensorData.space1 = digitalRead(IR_SENSOR_PIN1) == 0 ? "occupied" : "available";
            sensorData.space2 = digitalRead(IR_SENSOR_PIN2) == 0 ? "occupied" : "available";
            sensorData.space3 = digitalRead(IR_SENSOR_PIN3) == 0 ? "occupied" : "available";
            sensorData.space4 = digitalRead(IR_SENSOR_PIN4) == 0 ? "occupied" : "available";
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

// Task to send sensor data to the server
void sendDataToServer(void *parameter)
{
    while (1)
    {
        String jsonPayload;
        // Take the mutex to access shared sensor data
        if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
        {
            // Prepare JSON payload
            jsonPayload = "{";
            jsonPayload += "\"device_id\": \"" + String(deviceId) + "\",";
            jsonPayload += "\"parking_space_01\": \"" + sensorData.space1 + "\",";
            jsonPayload += "\"parking_space_02\": \"" + sensorData.space2 + "\",";
            jsonPayload += "\"parking_space_03\": \"" + sensorData.space3 + "\",";
            jsonPayload += "\"parking_space_04\": \"" + sensorData.space4 + "\"";
            jsonPayload += "}";
            xSemaphoreGive(xMutex);
        }

        // Send HTTP POST request if Wi-Fi is connected
        if (WiFi.status() == WL_CONNECTED)
        {
            HTTPClient http;
            http.begin(serverUrl);
            http.addHeader("Content-Type", "application/json");

            int httpResponseCode = http.POST(jsonPayload);
            if (httpResponseCode > 0)
            {
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
                String response = http.getString();
                Serial.println(response);
            }
            else
            {
                Serial.print("Error code: ");
                Serial.println(httpResponseCode);
            }
            http.end();
        }
        else
        {
            Serial.println("Wi-Fi disconnected");
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // Delay for 2 seconds
    }
}

// New task to monitor the entry IR sensor and control the servo motor
void monitorEntrySensor(void *parameter)
{
    while (1)
    {
        // Check if the entry sensor detects an object.
        // Sensor outputs LOW (0) when an object is detected.
        if (digitalRead(ENTRY_SENSOR_PIN) == 0)
        {
            bool parkingFull = false;

            // Take the mutex to safely check the parking spaces status
            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
            {
                if (sensorData.space1 == "occupied" &&
                    sensorData.space2 == "occupied" &&
                    sensorData.space3 == "occupied" &&
                    sensorData.space4 == "occupied")
                {
                    parkingFull = true;
                }
                xSemaphoreGive(xMutex);
            }

            if (!parkingFull)
            {
                Serial.println("Entry sensor triggered: Activating servo barrier.");
                // Rotate servo to 180°
                myServo.write(180);
                // Wait for 5 seconds
                vTaskDelay(pdMS_TO_TICKS(5000));
                // Return servo to 90°
                myServo.write(90);
            }
            else
            {
                Serial.println("All parking spaces occupied. Servo will not activate.");
            }

            // Wait until the entry sensor is clear to avoid repeated triggering
            while (digitalRead(ENTRY_SENSOR_PIN) == 0)
            {
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Poll every 100 ms
    }
}

// Function to connect to Wi-Fi
void connectToWiFi()
{
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}