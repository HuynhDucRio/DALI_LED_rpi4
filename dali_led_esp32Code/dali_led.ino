#define DALI_INPUT_PIN 16  // GPIO của ESP32 nhận tín hiệu từ Raspberry Pi
#define DALI_OUTPUT_PIN 17  // Chân GPIO 17 sẽ dùng để gửi tín hiệu

#define LED1_PIN 15
#define LED2_PIN 2
#define LED3_PIN 4
#define LED4_PIN 5
#define LED5_PIN 18
#define LED6_PIN 19
#define LED7_PIN 21
#define LED8_PIN 22

int lastBit = 1;           // Khởi tạo là 1 vì tín hiệu idle là 1
int bitCount = 0;          // Đếm số lượng bit đã nhận
bool isReceiving = false;   // Cờ để xác định đang nhận dữ liệu
String receivedString = ""; // Chuỗi để lưu dữ liệu nhận được
String addressBits = "";    // Biến để lưu địa chỉ dưới dạng nhị phân
String dataBits = "";       // Biến để lưu dữ liệu dưới dạng nhị phân

int data_temp = 0;

void setup() {
  Serial.begin(9600);
  // Đặt chân nhận tín hiệu là input
  pinMode(DALI_INPUT_PIN, INPUT);  
  // Đặt chân các led là output
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(LED5_PIN, OUTPUT);
  pinMode(LED6_PIN, OUTPUT);
  pinMode(LED7_PIN, OUTPUT);
  pinMode(LED8_PIN, OUTPUT);

  // Đặt chân OUTPUT_PIN là output để gửi tín hiệu đi
  pinMode(DALI_OUTPUT_PIN, OUTPUT);
}

String convertBinary(const String& input) {
    String output = ""; // Chuỗi đầu ra để lưu kết quả

    // Kiểm tra nếu độ dài chuỗi đầu vào là chẵn
    if (input.length() % 2 != 0) {
        Serial.println("Chuỗi đầu vào phải có độ dài chẵn.");
        return output; // Trả về chuỗi rỗng nếu không đúng định dạng
    }

    // Xử lý từng cặp bit
    for (size_t i = 0; i < input.length(); i += 2) {
        String pair = input.substring(i, i + 2); // Lấy 2 bit

        if (pair == "10") {
            output += "1"; // Nếu cặp là 10, thêm 1 vào đầu ra
        } else if (pair == "01") {
            output += "0"; // Nếu cặp là 01, thêm 0 vào đầu ra
        } else {
            Serial.println("Cặp bit không hợp lệ: " + pair);
        }
    }

    return output; // Trả về chuỗi kết quả
}

unsigned long binaryToDecimal(const String& binary) {
    unsigned long decimalValue = 0; // Biến lưu giá trị số nguyên
    unsigned long base = 1;          // Cơ số cho từng bit (2^0, 2^1, ...)

    // Xử lý từng bit từ cuối đến đầu
    for (int i = binary.length() - 1; i >= 0; i--) {
        // Nếu bit là '1', cộng giá trị cơ số vào decimalValue
        if (binary.charAt(i) == '1') {
            decimalValue += base;
        }
        base *= 2; // Tăng cơ số lên gấp đôi cho bit tiếp theo
    }

    return decimalValue; // Trả về giá trị số nguyên
}

void loop() {
  // Sử dụng vòng lặp while để nhận dữ liệu
  while (true) {
    int currentBit = digitalRead(DALI_INPUT_PIN); // Đọc bit hiện tại từ GPIO

    // Phát hiện chuỗi mở đầu "10" để bắt đầu nhận dữ liệu
    if (!isReceiving && lastBit == 1 && currentBit == 0) {
      isReceiving = true;  // Bắt đầu nhận dữ liệu
      bitCount = 0;        // Đặt lại số đếm bit
      receivedString = ""; // Xóa dữ liệu cũ trong chuỗi
      Serial.println("Start receiving data...");
    }

    // Nếu đang nhận dữ liệu, lưu bit vào chuỗi
    if (isReceiving) {
      // Thêm bit vào chuỗi
      receivedString += String(currentBit);

      // Debug từng bit nhận được
      Serial.print("Received bit: ");
      Serial.println(currentBit);

      bitCount++;

      // Khi đã nhận đủ 38 bit, kiểm tra 4 bit cuối
      if (bitCount == 38) {
        // Kiểm tra nếu 4 bit cuối là 1111
        bool endSequenceDetected = (receivedString.endsWith("1111"));

        if (endSequenceDetected) {
          Serial.println("Data received successfully:");
          
          // Bỏ qua bit đầu tiên và 4 bit cuối
          receivedString = receivedString.substring(1, receivedString.length() - 4); // Bỏ 4 bit cuối

          // Trích xuất địa chỉ và dữ liệu
          addressBits = receivedString.substring(0, 16); // 16 bit cho địa chỉ
          dataBits = receivedString.substring(16, 32);   // 16 bit cho dữ liệu

          Serial.print("Address (binary): ");
          Serial.println(addressBits);
          Serial.print("Data (binary): ");
          Serial.println(dataBits);
          
        } else {
          Serial.println("End sequence not detected. Discarding data.");
        }

        // Reset lại để chuẩn bị cho lần nhận tiếp theo
        isReceiving = false;
        break; // Thoát khỏi vòng lặp while sau khi nhận xong
      }
    }

    lastBit = currentBit;  // Cập nhật trạng thái bit cuối cùng
    delay(100);              // Điều chỉnh thời gian delay để đảm bảo lấy mẫu đủ nhanh
  }


    int address = binaryToDecimal(convertBinary(addressBits));
    Serial.println(address);
    int data = binaryToDecimal(convertBinary(dataBits));
    Serial.println(data);

    // Cấu hình các lệnh điều khiển led
    switch (address)
    {
    case 0x00:
        analogWrite(LED1_PIN, data);
        data_temp = data;
        break;

    case 0x02:
        analogWrite(LED2_PIN, data);
        data_temp = data;
        break;
    case 0x04:
        analogWrite(LED3_PIN, data);
        data_temp = data;
        break;

    case 0x06:
        analogWrite(LED4_PIN, data);
        data_temp = data;
        break;
    case 0x08:
        analogWrite(LED5_PIN, data);
        data_temp = data;
        break;

    case 0x0A:
        analogWrite(LED6_PIN, data);
        data_temp = data;
        break;
    case 0x0C:
        analogWrite(LED7_PIN, data);
        data_temp = data;
        break;

    case 0x0E:
        analogWrite(LED8_PIN, data);
        data_temp = data;
        break;
    case 0xFE:
        analogWrite(LED1_PIN, data);
        analogWrite(LED2_PIN, data);
        analogWrite(LED3_PIN, data);
        analogWrite(LED4_PIN, data);
        analogWrite(LED5_PIN, data);
        analogWrite(LED6_PIN, data);
        analogWrite(LED7_PIN, data);
        analogWrite(LED8_PIN, data);
        data_temp = data;
        break;
    case 0xFD:
        while(data != 0){
        digitalWrite(LED1_PIN, 1);
        digitalWrite(LED2_PIN, 1);
        digitalWrite(LED3_PIN, 1);
        digitalWrite(LED4_PIN, 1);
        digitalWrite(LED5_PIN, 1);
        digitalWrite(LED6_PIN, 1);
        digitalWrite(LED7_PIN, 1);
        digitalWrite(LED8_PIN, 1);
        delay(500);
        digitalWrite(LED1_PIN, 0);
        digitalWrite(LED2_PIN, 0);
        digitalWrite(LED3_PIN, 0);
        digitalWrite(LED4_PIN, 0);
        digitalWrite(LED5_PIN, 0);
        digitalWrite(LED6_PIN, 0);
        digitalWrite(LED7_PIN, 0);
        digitalWrite(LED8_PIN, 0);
        delay(500);
        data -= 1;
        }
        data_temp = data;
        break;
    case 0xFF:
        if(data_temp > data){
          while(data_temp != data - 1){
            analogWrite(LED1_PIN, data_temp);
            analogWrite(LED2_PIN, data_temp);
            analogWrite(LED3_PIN, data_temp);
            analogWrite(LED4_PIN, data_temp);
            analogWrite(LED5_PIN, data_temp);
            analogWrite(LED6_PIN, data_temp);
            analogWrite(LED7_PIN, data_temp);
            analogWrite(LED8_PIN, data_temp);
            data_temp -= 1;
            delay(20);
          }
        } else if (data_temp < data) {
          while(data_temp != data + 1){
            analogWrite(LED1_PIN, data_temp);
            analogWrite(LED2_PIN, data_temp);
            analogWrite(LED3_PIN, data_temp);
            analogWrite(LED4_PIN, data_temp);
            analogWrite(LED5_PIN, data_temp);
            analogWrite(LED6_PIN, data_temp);
            analogWrite(LED7_PIN, data_temp);
            analogWrite(LED8_PIN, data_temp);
            data_temp += 1;
            delay(20);
          }
        }
        break;
      case 0x82:
          analogWrite(LED1_PIN, data);
          analogWrite(LED2_PIN, data);
          analogWrite(LED3_PIN, data);
          analogWrite(LED4_PIN, data);
          data_temp = data;
          break;
      case 0x84:
          analogWrite(LED5_PIN, data);
          analogWrite(LED6_PIN, data);
          analogWrite(LED7_PIN, data);
          analogWrite(LED8_PIN, data);
          data_temp = data;
          break;
      case 0x86:
          analogWrite(LED1_PIN, 0);
          analogWrite(LED2_PIN, 0);
          analogWrite(LED3_PIN, data);
          analogWrite(LED4_PIN, data);
          analogWrite(LED5_PIN, data);
          analogWrite(LED6_PIN, data);
          analogWrite(LED7_PIN, 0);
          analogWrite(LED8_PIN, 0);
          data_temp = data;
          break;
    default:
        printf("Default case is Matched.");
        break;
    }

   
    


  // Thực hiện các tác vụ khác tại đây, nếu cần
  if (data > 0) {
    digitalWrite(DALI_OUTPUT_PIN, HIGH);  // Gửi tín hiệu 1
  } else {
    digitalWrite(DALI_OUTPUT_PIN, LOW);   // Gửi tín hiệu 0
  }
}