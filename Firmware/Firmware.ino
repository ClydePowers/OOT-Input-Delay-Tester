/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OOT Input Lag Tester
// Written by ClydePowers (https://github.com/ClydePowers)
// Original Code written by jaburns (https://github.com/jaburns/NintendoSpy)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Press A+B to reset Counter and Avg. Delay
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int BRIGHTNESS = 250; // Value depends on Photodiode/transistor/resistor, Resistor and the
                      // Brightness of your Monitor
int NUTDELAY = 250; // Throwing a Deku Nut in front off a wall takes 5frames until white flash
long avgDelay = 0;
long totalTime = 0;
int counter = 0;
unsigned long delayMS;
unsigned long timeOnPress;
unsigned long currentTime;
String output;
String merkel = "#";
unsigned char rawData[128];

// ---------- Uncomment one of these options to select operation mode --------------
#define MODE_GC
//#define MODE_N64

#define PIN_READ(pin) (PIND & (1 << (pin)))
#define PINC_READ(pin) (PINC & (1 << (pin)))
#define MICROSECOND_NOPS                                                                           \
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
#define WAIT_FALLING_EDGE(pin)                                                                     \
    while (!PIN_READ(pin))                                                                         \
        ;                                                                                          \
    while (PIN_READ(pin))                                                                          \
        ;

#define MODEPIN_N64 1
#define MODEPIN_GC 2

#define N64_PIN 7
#define N64_PREFIX 9
#define N64_BITCOUNT 32

#define GC_PIN 5
#define GC_PREFIX 25
#define GC_BITCOUNT 64

#define ZERO '\0' // Use a byte value of 0x00 to represent a bit with value 0.
#define ONE                                                                                        \
    '1' // Use an ASCII one to represent a bit with value 1.  This makes Arduino debugging easier.
#define SPLIT '\n' // Use a new-line character to split up the controller state packets.

void setup()
{
    PORTD = 0x00;
    DDRD = 0x00;
    PORTC = 0xFF; // Set the pull-ups on the port we use to check operation mode.
    DDRC = 0x00;
    Serial.begin(1000000);
}

template <unsigned char pin>
void read_oneWire(unsigned char bits)
{
    unsigned char* rawDataPtr = rawData;

read_loop:

    WAIT_FALLING_EDGE(pin);

    asm volatile(MICROSECOND_NOPS MICROSECOND_NOPS);

    *rawDataPtr = PIN_READ(pin);
    ++rawDataPtr;
    if (--bits == 0)
        return;

    goto read_loop;
}

inline bool checkPrefixN64()
{
    if (rawData[0] != 0)
        return false; // 0
    if (rawData[1] != 0)
        return false; // 0
    if (rawData[2] != 0)
        return false; // 0
    if (rawData[3] != 0)
        return false; // 0
    if (rawData[4] != 0)
        return false; // 0
    if (rawData[5] != 0)
        return false; // 0
    if (rawData[6] != 0)
        return false; // 0
    if (rawData[7] == 0)
        return false; // 1
    if (rawData[8] == 0)
        return false; // 1
    return true;
}

template <unsigned char latch, unsigned char data, unsigned char longWait>
void read_shiftRegister_2wire(unsigned char bits)
{
    unsigned char* rawDataPtr = rawData;

    WAIT_FALLING_EDGE(latch);

read_loop:

    // Read the data from the line and store in "rawData"
    *rawDataPtr = !PIN_READ(data);
    ++rawDataPtr;
    if (--bits == 0)
        return;

    // Wait until the next button value is on the data line. ~12us between each.
    asm volatile(
        MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS
            MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS);
    if (longWait)
    {
        asm volatile(MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS
                MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS MICROSECOND_NOPS
                    MICROSECOND_NOPS);
    }

    goto read_loop;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Preferred method for reading SNES + NES controller data.
template <unsigned char latch, unsigned char data, unsigned char clock>
void read_shiftRegister(unsigned char bits)
{
    unsigned char* rawDataPtr = rawData;

    WAIT_FALLING_EDGE(latch);

    do
    {
        WAIT_FALLING_EDGE(clock);
        *rawDataPtr = !PIN_READ(data);
        ++rawDataPtr;
    } while (--bits > 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sends a packet of controller data over the Arduino serial interface.
inline void sendRawData(unsigned char first, unsigned char count)
{
    for (unsigned char i = first; i < first + count; i++)
    {
        Serial.write(rawData[i] ? ONE : ZERO);
    }
    Serial.write(SPLIT);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update loop definitions for the various console modes.

inline void loop_GC()
{
    noInterrupts();
    read_oneWire<GC_PIN>(GC_PREFIX + GC_BITCOUNT);
    interrupts();

    if ((rawData[GC_PREFIX + 4] || rawData[GC_PREFIX + 5] || rawData[GC_PREFIX + 11]))
    {
        currentTime = millis();
        timeOnPress = currentTime;

        while (analogRead(A3) < BRIGHTNESS && currentTime - timeOnPress < 5000)
        {
            currentTime = millis();
        }

        delayMS = currentTime - timeOnPress - NUTDELAY;
        counter++;
        totalTime += delayMS;
        avgDelay = totalTime / counter;
        output = "Input Delay: ";
        output
            = merkel + counter + " " + output + delayMS + "ms   Average Delay: " + avgDelay + "ms";
        Serial.println(output);
    }

    if (rawData[GC_PREFIX + 8] && rawData[GC_PREFIX + 7])
    {
        counter = 0;
        totalTime = 0;
        avgDelay = 0;
    }
}

inline void loop_N64()
{
    noInterrupts();
    read_oneWire<N64_PIN>(N64_PREFIX + N64_BITCOUNT);
    interrupts();
    if (checkPrefixN64())
    {


        // code here
        // instead of report.x it will have to be rawData[]; it will be N64_PREFIX + whatever bit C
        // button is.


        // sendRawData( N64_PREFIX , N64_BITCOUNT ); // dont need this anymore
    }
}

void loop()
{
#ifdef MODE_GC
    loop_GC();
#elif defined MODE_N64
    loop_N64();
#elif defined MODE_SNES
    loop_SNES();
#elif defined MODE_NES
    loop_NES();
#elif defined MODE_DETECT
    if (!PINC_READ(MODEPIN_SNES))
    {
        loop_SNES();
    }
    else if (!PINC_READ(MODEPIN_N64))
    {
        loop_N64();
    }
    else if (!PINC_READ(MODEPIN_GC))
    {
        loop_GC();
    }
    else
    {
        loop_NES();
    }
#endif
}
