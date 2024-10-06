#define RXD2 13
#define TXD2 2


const char * GNRMC_MESSAGE_HEADER = "$GNRMC,"; // current implementation needs comma to be included into header

// $GNRMC (Minimum GNSS Data)
struct {
  char gnss_data[80];
  bool GetData_Flag;    // Get GNSS data flag bit
  bool ParseData_Flag;  // Parse completed flag bit
  char UTCTime[11];     // UTC time
  float latitude;    // Latitude
  char N_S[2];          // N/S
  float longitude;   // Longitude
  char E_W[2];          // E/W
  float speed_over_ground;
  float course_over_ground;
  bool Usefull_Flag;    // If the position information is valid flag bit
} parsed_gnss_data;

const unsigned int rx_buffer_length = 600;
char rx_buffer[rx_buffer_length];
unsigned int rx_buffer_pointer = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("My GNSS");
  Serial.println("Wating...");

  parsed_gnss_data.GetData_Flag = false;
  parsed_gnss_data.ParseData_Flag = false;
  parsed_gnss_data.Usefull_Flag = false;
}

void loop() {
  read_raw_gnss_data(GNRMC_MESSAGE_HEADER);  // Get GNSS data
  parse_gnss_data();  // Analyze GNSS data
  print_gnss_data();  // Output analyzed data
}

void error_flag(int num) {
  Serial.print("ERROR");
  Serial.println(num);
  while (1) {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
  }
}

void print_gnss_data() {

  if (parsed_gnss_data.ParseData_Flag) {
    parsed_gnss_data.ParseData_Flag = false;

    Serial.println("************************");
    Serial.println(parsed_gnss_data.gnss_data);

    
    Serial.print("parsed_gnss_data.UTCTime = ");
    Serial.println(parsed_gnss_data.UTCTime);
    if (parsed_gnss_data.Usefull_Flag) {
      parsed_gnss_data.Usefull_Flag = false;
      Serial.print("parsed_gnss_data.latitude = ");
      Serial.printf("%.5f\n",parsed_gnss_data.latitude);
      Serial.print("parsed_gnss_data.N_S = ");
      Serial.println(parsed_gnss_data.N_S);
      Serial.print("parsed_gnss_data.longitude = ");
      Serial.printf("%.5f\n",parsed_gnss_data.longitude);
      Serial.print("parsed_gnss_data.E_W = ");
      Serial.println(parsed_gnss_data.E_W);
      Serial.print("parsed_gnss_data.speed_over_ground = ");
      Serial.printf("%.2f\n",parsed_gnss_data.speed_over_ground);
      Serial.print("parsed_gnss_data.course_over_ground = ");
      Serial.printf("%.2f\n",parsed_gnss_data.course_over_ground);
    } else {
      Serial.println("GNSS DATA is not usefull!");
    }
  }
}

void parse_gnss_data() {
  char* subString;
  char* subStringNext;
  if (!parsed_gnss_data.GetData_Flag) {
    return;
  }

  parsed_gnss_data.GetData_Flag = false;

  for (int i = 0; i <= 8; i++) {
    if (i == 0) {
      if ((subString = strstr(parsed_gnss_data.gnss_data, ",")) == NULL){
        error_flag(1);  // Analysis error
      }
      continue;
    }

    subString++;
    if ((subStringNext = strstr(subString, ",")) == NULL) {
      error_flag(2);  // Analysis error
      continue;
    }

    char usefullBuffer[2];
    switch (i) {
      case 1:
        memcpy(parsed_gnss_data.UTCTime, subString, subStringNext - subString);
        break;  // Get UTC time
      case 2:
        memcpy(usefullBuffer, subString, subStringNext - subString);
        break;  // Get position status
      case 3:
        char lat_buffer_deg[3];
        char lat_buffer_min[8];
        memcpy(lat_buffer_deg, subString, 2);
        memcpy(lat_buffer_min, subString + 2, 7);

        parsed_gnss_data.latitude = atof(lat_buffer_deg) + atof(lat_buffer_min)/60;
        break;  // Get latitude information
      case 4:
        memcpy(parsed_gnss_data.N_S, subString, subStringNext - subString);
        break;  // Get N/S
      case 5:
        char lon_buffer_deg[4];
        char lon_buffer_min[8];
        memcpy(lon_buffer_deg, subString, 3);
        memcpy(lon_buffer_min, subString + 3, 7);

        parsed_gnss_data.longitude = atof(lon_buffer_deg) + atof(lon_buffer_min)/60;
        break;  // Get longitude information
      case 6:
        memcpy(parsed_gnss_data.E_W, subString, subStringNext - subString);
        break;  // Get E/W
      case 7:
        char speed_over_ground_buffer[5];
        memcpy(speed_over_ground_buffer, subString, 4);

        parsed_gnss_data.speed_over_ground = atof(speed_over_ground_buffer);
        break;  // Get speed over ground
      case 8:
        char course_over_ground_buffer[7];
        memcpy(course_over_ground_buffer, subString, 6);
        parsed_gnss_data.course_over_ground = atof(course_over_ground_buffer);
        break;  // Get course over ground

      default:
        break;
    }
    subString = subStringNext;
    parsed_gnss_data.ParseData_Flag = true;
    if (usefullBuffer[0] == 'A')
      parsed_gnss_data.Usefull_Flag = true;
    else if (usefullBuffer[0] == 'V')
      parsed_gnss_data.Usefull_Flag = false;
  }
}


void read_raw_gnss_data(const char * message_header) 
{
  while (Serial2.available())
  {
    rx_buffer[rx_buffer_pointer++] = Serial2.read();
    if (rx_buffer_pointer == rx_buffer_length) reset_gnss_buffer();
  }

  char* gnss_data_head;
  char* gnss_data_tail;

  if ((gnss_data_head = strstr(rx_buffer, message_header)) == NULL ) return;
  if (((gnss_data_tail = strstr(gnss_data_head, "\r\n")) == NULL) || (gnss_data_tail <= gnss_data_head)) return;

  memcpy(parsed_gnss_data.gnss_data, gnss_data_head, gnss_data_tail - gnss_data_head);
  parsed_gnss_data.GetData_Flag = true;

  reset_gnss_buffer();

  
}

void reset_gnss_buffer(void) {
  memset(rx_buffer, 0, rx_buffer_length);  // Clear
  rx_buffer_pointer = 0;
}
