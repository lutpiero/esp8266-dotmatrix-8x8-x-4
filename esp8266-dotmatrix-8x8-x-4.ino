namespace {
constexpr uint8_t kDataPin = 13;
constexpr uint8_t kClockPin = 14;
constexpr uint8_t kLoadPin = 15;
constexpr uint8_t kModuleCount = 4;
constexpr uint16_t kFrameDelayMs = 350;

const uint8_t kFrames[][8][kModuleCount] = {
    {
        {0x00, 0x3C, 0x3C, 0x00},
        {0x42, 0x42, 0x42, 0x42},
        {0xA5, 0x81, 0x81, 0xA5},
        {0x81, 0x81, 0x81, 0x81},
        {0xA5, 0x81, 0x81, 0xA5},
        {0x99, 0x42, 0x42, 0x99},
        {0x42, 0x3C, 0x3C, 0x42},
        {0x00, 0x00, 0x00, 0x00},
    },
    {
        {0x00, 0x18, 0x18, 0x00},
        {0x3C, 0x24, 0x24, 0x3C},
        {0x7E, 0x42, 0x42, 0x7E},
        {0xDB, 0x81, 0x81, 0xDB},
        {0xFF, 0x81, 0x81, 0xFF},
        {0x66, 0x42, 0x42, 0x66},
        {0x3C, 0x24, 0x24, 0x3C},
        {0x00, 0x18, 0x18, 0x00},
    },
};

void writeCommand(uint8_t address, uint8_t value) {
  digitalWrite(kLoadPin, LOW);
  for (uint8_t module = 0; module < kModuleCount; ++module) {
    shiftOut(kDataPin, kClockPin, MSBFIRST, address);
    shiftOut(kDataPin, kClockPin, MSBFIRST, value);
  }
  digitalWrite(kLoadPin, HIGH);
}

void writeRow(uint8_t row, const uint8_t values[kModuleCount]) {
  digitalWrite(kLoadPin, LOW);
  for (uint8_t module = 0; module < kModuleCount; ++module) {
    shiftOut(kDataPin, kClockPin, MSBFIRST, row + 1);
    shiftOut(kDataPin, kClockPin, MSBFIRST, values[module]);
  }
  digitalWrite(kLoadPin, HIGH);
}

void clearDisplay() {
  const uint8_t empty[kModuleCount] = {0};
  for (uint8_t row = 0; row < 8; ++row) {
    writeRow(row, empty);
  }
}

void showFrame(uint8_t frameIndex) {
  for (uint8_t row = 0; row < 8; ++row) {
    writeRow(row, kFrames[frameIndex][row]);
  }
}
}  // namespace

void setup() {
  pinMode(kDataPin, OUTPUT);
  pinMode(kClockPin, OUTPUT);
  pinMode(kLoadPin, OUTPUT);

  writeCommand(0x0F, 0x00);
  writeCommand(0x09, 0x00);
  writeCommand(0x0B, 0x07);
  writeCommand(0x0A, 0x03);
  writeCommand(0x0C, 0x01);
  clearDisplay();
}

void loop() {
  static uint8_t frameIndex = 0;

  showFrame(frameIndex);
  frameIndex = (frameIndex + 1) % (sizeof(kFrames) / sizeof(kFrames[0]));
  delay(kFrameDelayMs);
}
