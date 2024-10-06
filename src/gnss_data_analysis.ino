#define RXD2 13
#define TXD2 2

struct {
  char gnss_data_[80];
  bool GetData_Flag;    // Get GNSS data flag bit
  bool ParseData_Flag;  // Parse completed flag bit
  char UTCTime[11];     // UTC time
  char latitude[11];    // Latitude
  char N_S[2];          // N/S
  char longitude[12];   // Longitude
  char E_W[2];          // E/W
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
  read_raw_gnss_data_();  // Get GNSS data
  parse_gnss_data_();  // Analyze GNSS data
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

    Serial.print("parsed_gnss_data.UTCTime = ");
    Serial.println(parsed_gnss_data.UTCTime);

    if (parsed_gnss_data.Usefull_Flag) {
      parsed_gnss_data.Usefull_Flag = false;
      Serial.print("parsed_gnss_data.latitude = ");
      Serial.println(parsed_gnss_data.latitude);
      Serial.print("parsed_gnss_data.N_S = ");
      Serial.println(parsed_gnss_data.N_S);
      Serial.print("parsed_gnss_data.longitude = ");
      Serial.println(parsed_gnss_data.longitude);
      Serial.print("parsed_gnss_data.E_W = ");
      Serial.println(parsed_gnss_data.E_W);
    } else {
      Serial.println("GNSS DATA is not usefull!");
    }
  }
}

void parse_gnss_data_() {
  char* subString;
  char* subStringNext;
  if (!parsed_gnss_data.GetData_Flag) {
    return;
  }

  parsed_gnss_data.GetData_Flag = false;
  Serial.println("************************");
  Serial.println(parsed_gnss_data.gnss_data_);

  for (int i = 0; i <= 6; i++) {
    if (i == 0) {
      if ((subString = strstr(parsed_gnss_data.gnss_data_, ",")) == NULL){
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
        memcpy(parsed_gnss_data.latitude, subString, subStringNext - subString);
        break;  // Get latitude information
      case 4:
        memcpy(parsed_gnss_data.N_S, subString, subStringNext - subString);
        break;  // Get N/S
      case 5:
        memcpy(parsed_gnss_data.longitude, subString, subStringNext - subString);
        break;  // Get longitude information
      case 6:
        memcpy(parsed_gnss_data.E_W, subString, subStringNext - subString);
        break;  // Get E/W

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


void read_raw_gnss_data_() 
{
  while (Serial2.available())
  {
    rx_buffer[rx_buffer_pointer++] = Serial2.read();
    if (rx_buffer_pointer == rx_buffer_length) reset_gnss_buffer();
  }

  char* gnss_data_head;
  char* gnss_data_tail;

  if ((gnss_data_head = strstr(rx_buffer, "$GNRMC,")) == NULL ) return;
  if (((gnss_data_tail = strstr(gnss_data_head, "\r\n")) == NULL) || (gnss_data_tail <= gnss_data_head)) return;

  memcpy(parsed_gnss_data.gnss_data_, gnss_data_head, gnss_data_tail - gnss_data_head);
  parsed_gnss_data.GetData_Flag = true;

  reset_gnss_buffer();

  
}

void reset_gnss_buffer(void) {
  memset(rx_buffer, 0, rx_buffer_length);  // Clear
  rx_buffer_pointer = 0;
}
