typedef unsigned char uint8_t;

const uint8_t CMD_NOOP = 0;
const uint8_t CMD_SETUP = 1;
const uint8_t CMD_GETPORT = 2;
const uint8_t CMD_SETPORT = 3;
const uint8_t CMD_GETOPTION = 4;
const uint8_t CMD_SETOPTION = 5;
const uint8_t CMD_SETSINGLE = 6;
const uint8_t CMD_DELSINGLE = 7;
const uint8_t CMD_TOGGLE = 8;

const uint8_t CMD_LEN = 4u;

const uint8_t RSP_NOOP = 255;
const uint8_t RSP_SETUP = 254;
const uint8_t RSP_GETPORT = 253;
const uint8_t RSP_SETPORT = 253;
const uint8_t RSP_GETOPTION = 251;
const uint8_t RSP_SETOPTION = 250;
const uint8_t RSP_SETSINGLE = 249;
const uint8_t RSP_DELSINGLE = 248;
const uint8_t RSP_TOGGLE = 247;

const uint8_t RSP_LEN = 4u;

const uint8_t OFFS_CMD = 0;
const uint8_t OFFS_RSP = 0;
const uint8_t OFFS_ADDR = 1;
const uint8_t OFFS_DATA = 2;
const uint8_t OFFS_XOR = 3;

static uint8_t const gVersion = 0;
static uint8_t gCurrentAddr = 0;

const uint8_t FIRST_PIN = 2;
const uint8_t MAX_PIN_CNT = 8;

void _setPinMode(uint8_t pin, uint8_t mode)
{
    pinMode(pin + FIRST_PIN, mode);
}

void _writePin(uint8_t pin, uint8_t mode)
{
    digitalWrite(pin + FIRST_PIN, mode);
}

uint8_t _readPin(uint8_t pin)
{
    return digitalRead(pin + FIRST_PIN);
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < MAX_PIN_CNT; ++i)
  {
    _writePin(i, LOW);
    _setPinMode(i, OUTPUT);
  }
  Serial.begin(9600);
}

uint8_t get_xor(const uint8_t *const data, uint8_t data_len)
{
  if (data_len < 4)
  {
    return 0U;
  }

  return data[OFFS_CMD] ^ data[OFFS_ADDR] ^ data[OFFS_DATA];
}

bool check_xor(const uint8_t *const data, uint8_t data_len)
{
  if (data_len < 4)
  {
    return false; 
  }
  uint8_t const crc = data[3];
  uint8_t const calc_crc = get_xor(data, data_len);
  return (crc == calc_crc);
}

void send_resp(uint8_t resp, uint8_t data)
{
  uint8_t frame[RSP_LEN];
  frame[OFFS_RSP] = resp;
  frame[OFFS_ADDR] = gCurrentAddr;
  frame[OFFS_DATA] = data;
  const uint8_t crc = get_xor(frame, sizeof(frame));
  frame[OFFS_XOR] = crc;
  Serial.write(frame, sizeof(frame));
  Serial.flush();
}

void cmd_noop(const uint8_t* const data, uint8_t data_len)
{
  send_resp(RSP_NOOP, 0);
}

void cmd_setup(const uint8_t* const data, uint8_t data_len)
{
  send_resp(RSP_SETUP, gVersion);
}

uint8_t read_port()
{
  uint8_t port = 0;
  uint8_t pin;
  for (int i = 0; i < MAX_PIN_CNT; ++i)
  {
    pin = (_readPin(i)==LOW)? 0 : 1;
    port |= ((pin << i));
  }
  return port;
}

void cmd_getport(const uint8_t* const data, uint8_t data_len)
{
  const uint8_t port = read_port();
  send_resp(RSP_GETPORT, port);
}

void cmd_setport(const uint8_t* const data, uint8_t data_len)
{
  const uint8_t port = data[OFFS_DATA];
  uint8_t pin;
  for (int i = 0; i < MAX_PIN_CNT; ++i)
  {
    pin = ((port & (1 << i)) == 0) ? LOW : HIGH;
    _writePin(i, pin);
  }
  send_resp(RSP_SETPORT, 0);
}

void cmd_getoption(const uint8_t* const data, uint8_t data_len)
{
  send_resp(RSP_NOOP, 0);
}

void cmd_setoption(const uint8_t* const data, uint8_t data_len)
{
  send_resp(RSP_NOOP, 0);
}

void cmd_setsingle(const uint8_t* const data, uint8_t data_len)
{
  const uint8_t port = data[OFFS_DATA]; 
  uint8_t pin;
  for (int i = 0; i < MAX_PIN_CNT; ++i)
  {
    if ((port & (1 << i)) != 0)
    {
      _writePin(i, HIGH);
    }
  }
  send_resp(RSP_SETSINGLE, read_port());
}

void cmd_delsingle(const uint8_t* const data, uint8_t data_len)
{
  const uint8_t port = data[OFFS_DATA]; 
  for (int i = 0; i < MAX_PIN_CNT; ++i)
  {
    if ((port & (1 << i)) != 0)
    {
      _writePin(i, LOW);
    }
  }
  send_resp(RSP_DELSINGLE, read_port());
}

void cmd_toggle(const uint8_t* const data, uint8_t data_len)
{
  const uint8_t port = (~read_port()) & data[OFFS_DATA];
  uint8_t pin;
  for (int i = 0; i < MAX_PIN_CNT; ++i)
  {
    if (data[OFFS_DATA] & (1 << i))
    {
      pin = ((port & (1 << i)) == 0) ? LOW : HIGH;
      _writePin(i, pin);
    }
  }
  send_resp(RSP_TOGGLE, read_port());
}


void cmd(const uint8_t *const data, uint8_t data_len)
{
  if (data_len < 4)
  {
    return;
  }

  const uint8_t cmd = data[OFFS_CMD];
  switch(cmd)
  {
    case CMD_NOOP:
        cmd_noop(data, data_len);
        break;
    case CMD_SETUP:
        cmd_setup(data, data_len);
        break;
    case CMD_GETPORT:
        cmd_getport(data, data_len);
        break;
    case CMD_SETPORT:
        cmd_setport(data, data_len);
        break;
    case CMD_GETOPTION:
        cmd_getoption(data, data_len);
        break;
    case CMD_SETOPTION:
        cmd_setoption(data, data_len);
        break;
    case CMD_SETSINGLE:
        cmd_setsingle(data, data_len);
        break;
    case CMD_DELSINGLE:
        cmd_delsingle(data, data_len);
        break;
    case CMD_TOGGLE:
        cmd_toggle(data, data_len);
        break;
    default:
        cmd_noop(data, data_len);
        break;
  }
}

// the loop function runs over and over again forever
void loop() {

  static uint8_t buffer[4];

  const uint8_t read = Serial.readBytes(buffer, sizeof(buffer));
  if (read == 4) 
  {
      cmd(buffer, read);
  }
}