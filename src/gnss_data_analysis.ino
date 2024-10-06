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
} Save_Data;

const unsigned int GNSSRxBufferLength = 600;
char GNSSRxBuffer[GNSSRxBufferLength];
unsigned int GNSSRxLength = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("My GNSS");
  Serial.println("Wating...");

  Save_Data.GetData_Flag = false;
  Save_Data.ParseData_Flag = false;
  Save_Data.Usefull_Flag = false;
}

void loop() {
  read_raw_gnss_data_();       // Get GNSS data
  parse_gnss_data_();  // Analyze GNSS data
  print_GNSSDATA();  // Output analyzed data
}

void Error_Flag(int num) {
  Serial.print("ERROR");
  Serial.println(num);
  while (1) {
    digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
    delay(500);
  }
}

void print_GNSSDATA() {
  if (Save_Data.ParseData_Flag) {
    Save_Data.ParseData_Flag = false;

    Serial.print("Save_Data.UTCTime = ");
    Serial.println(Save_Data.UTCTime);

    if (Save_Data.Usefull_Flag) {
      Save_Data.Usefull_Flag = false;
      Serial.print("Save_Data.latitude = ");
      Serial.println(Save_Data.latitude);
      Serial.print("Save_Data.N_S = ");
      Serial.println(Save_Data.N_S);
      Serial.print("Save_Data.longitude = ");
      Serial.println(Save_Data.longitude);
      Serial.print("Save_Data.E_W = ");
      Serial.println(Save_Data.E_W);
    } else {
      Serial.println("GNSS DATA is not usefull!");
    }
  }
}

void parse_gnss_data_() {
  char* subString;
  char* subStringNext;
  if (!Save_Data.GetData_Flag) {
    return;
  }

  Save_Data.GetData_Flag = false;
  Serial.println("************************");
  Serial.println(Save_Data.gnss_data_);

  for (int i = 0; i <= 6; i++) {
    if (i == 0) {
      if ((subString = strstr(Save_Data.gnss_data_, ",")) == NULL){
        Error_Flag(1);  // Analysis error
      }
      continue;
    }

    subString++;
    if ((subStringNext = strstr(subString, ",")) == NULL) {
      Error_Flag(2);  // Analysis error
      continue;
    }

    char usefullBuffer[2];
    switch (i) {
      case 1:
        memcpy(Save_Data.UTCTime, subString, subStringNext - subString);
        break;  // Get UTC time
      case 2:
        memcpy(usefullBuffer, subString, subStringNext - subString);
        break;  // Get position status
      case 3:
        memcpy(Save_Data.latitude, subString, subStringNext - subString);
        break;  // Get latitude information
      case 4:
        memcpy(Save_Data.N_S, subString, subStringNext - subString);
        break;  // Get N/S
      case 5:
        memcpy(Save_Data.longitude, subString, subStringNext - subString);
        break;  // Get longitude information
      case 6:
        memcpy(Save_Data.E_W, subString, subStringNext - subString);
        break;  // Get E/W

      default:
        break;
    }
    subString = subStringNext;
    Save_Data.ParseData_Flag = true;
    if (usefullBuffer[0] == 'A')
      Save_Data.Usefull_Flag = true;
    else if (usefullBuffer[0] == 'V')
      Save_Data.Usefull_Flag = false;
    
    
  }
  
}


void read_raw_gnss_data_() 
{
  while (Serial2.available())
  {
    GNSSRxBuffer[GNSSRxLength++] = Serial2.read();
    if (GNSSRxLength == GNSSRxBufferLength)reset_gnss_buffer();
  }

  char* gnss_data_head;
  char* gnss_data_tail;

  if ((gnss_data_head = strstr(GNSSRxBuffer, "$GNRMC,")) == NULL ){
    return;
  }

  if (((gnss_data_tail = strstr(gnss_data_head, "\r\n")) == NULL) || (gnss_data_tail <= gnss_data_head)){
    return;
  }

  memcpy(Save_Data.gnss_data_, gnss_data_head, gnss_data_tail - gnss_data_head);
  Save_Data.GetData_Flag = true;

  reset_gnss_buffer();

  
}

void reset_gnss_buffer(void) {
  memset(GNSSRxBuffer, 0, GNSSRxBufferLength);  // Clear
  GNSSRxLength = 0;
}
