#include <ArduinoSTL.h>
using std::vector;
#include <OBD2.h>
#include <SD.h>
#include <SPI.h>

static const uint8_t SD_CS_PIN = 4;

// Will log all supported PIDs if no PIDs are provided.
vector<uint8_t> logged_pids({THROTTLE_POSITION, TIMING_ADVANCE});
File log_file;

static const bool VERBOSE = true;

void setup()
{

  Serial.begin(115200);

  // Wait for OBD2
  if (VERBOSE)
    serial_println(F("Waiting for OBD2..."));

  if (!OBD2.begin())
  {

    // Flash onboard LED if OBD2.begin() fails
    // OBD2 takes control of the LED pin so we need to release it
    OBD2.end();
    bool pin_status = false;
    while (true)
    {
      unsigned long start_time = millis();
      pin_status = !pin_status;
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, pin_status);

      // Attempt to reconnect to OBD2 if the LED is off.
      // This is a workaround for the OBD2 library not releasing the LED pin.
      if (!pin_status)
      {
        if (OBD2.begin())
        {
          if (VERBOSE)
            serial_println(F("Connected to OBD2"));

          break;
        }
        OBD2.end();
      }
      delay(500 - (millis() - start_time));
    }
  }

  // NOTE: Attempting to read VIN and ECU Name just hangs up the sketch, as `parsePacket` keeps returning `0` and getting stuck in an infinite loop.
  /*
  serial_print(F("VIN: "));
  serial_println(OBD2.vinRead());
  serial_print(F("ECU Name: "));
  serial_println(OBD2.ecuNameRead());
  */

  if (SD.begin(SPI_FULL_SPEED, SD_CS_PIN))
  {
    if (VERBOSE)
    {
      serial_println(F("SD card initialized. File logging enabled."));
    }
    log_file = SD.open("log_" + String(get_log_file_number()) + ".csv", FILE_WRITE);
  }

  // Log all supported PIDs if no pids are provided
  if (logged_pids.size() == 0)
  {
    // Get supported PIDs
    for (uint8_t pid = 1; pid <= 200; pid++)
    {
      if (OBD2.pidSupported(pid))
      {
        if (VERBOSE)
        {
          serial_print(F("Got supported PID: "));
          serial_println(OBD2.pidName(pid));
        }
        logged_pids.push_back(pid);
      }
      else
      {
        if (VERBOSE)
        {
          serial_print(F("Got unsupported PID: "));
          serial_println(OBD2.pidName(pid));
        }
      }
    }
  }

  serial_print(F("Will log "));
  serial_print(logged_pids.size());
  serial_println(F(" PIDs."));

  // Write header row to log file
  for (auto iter = logged_pids.begin(); iter != logged_pids.end(); ++iter)
  {
    String name = OBD2.pidName(*iter);
    log_file.print(name);
    // Skip the delimiter on the last PID
    if (iter + 1 != logged_pids.end())
    {
      log_file.print(',');
    }
  }
  log_file.println();
  log_file.flush();
}

void loop()
{
  unsigned long start_time = millis();
  for (auto iter = logged_pids.begin(); iter != logged_pids.end(); ++iter)
  {
    float value = OBD2.pidRead(*iter);
    log_file.print(value);
    serial_print(value);
    // Skip the delimiter on the last PID
    if (iter + 1 != logged_pids.end())
    {
      log_file.print(',');
      serial_print(',');
    }
  }
  log_file.println();
  serial_print("\r\n");
  log_file.flush();

  if (VERBOSE)
  {
    serial_print(F("Time taken: "));
    serial_print(millis() - start_time);
    serial_println("ms.");
  }
}

/**
 * Print a message to the serial monitor if it is available.
 * @param message The message to print.
 */
template <typename MessageType>
void serial_print(const MessageType &message)
{
  if (Serial)
    Serial.print(message);
}

/**
 * Print a message to the serial monitor if it is available and append a newline character.
 * @param message The message to print.
 */
template <typename MessageType>
void serial_println(const MessageType &message)
{
  if (Serial)
    Serial.println(message);
}

/**
 * Print a message to the log file if it is available.
 * @param message The message to print.
 */
template <typename MessageType>
void file_print(const MessageType &message)
{
  if (log_file)
    log_file.print(message);
}

/**
 * Print a message to the log file if it is available and append a newline character.
 * @param message The message to print.
 */
template <typename MessageType>
void file_println(const MessageType &message)
{
  if (log_file)
    log_file.println(message);
}

/**
 * Get the next available log file number.
 * @returns the next available log file number.
 */
int get_log_file_number()
{
  int log_file_number = 0;
  while (SD.exists("log_" + String(log_file_number) + ".csv"))
  {
    log_file_number++;
  }
  return log_file_number;
}
