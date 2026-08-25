#include "GPS.h"

// Helper function to convert a single hex character to its integer value
int hex2int(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    return 0; // Fallback for invalid characters
}

// --- Checksum Validator to prevent corrupted coordinates ---
bool validateNMEAChecksum(const char *line)
{
    if (line[0] != '$')
        return false;

    uint8_t calculated_checksum = 0;
    int i = 1;

    // XOR every byte between '$' and '*'
    while (line[i] != '*' && line[i] != '\0')
    {
        calculated_checksum ^= line[i];
        i++;
    }

    // If we found the asterisk, extract and compare the provided checksum
    if (line[i] == '*')
    {
        // Safe conversion without using heavy standard library functions
        int provided_checksum = (hex2int(line[i + 1]) << 4) | hex2int(line[i + 2]);
        return calculated_checksum == provided_checksum;
    }
    return false; // Malformed or missing checksum
}

// --- Helper: Convert NMEA raw format to Scaled Integer ---
int32_t nmeaToScaledDegrees(const char *nmea_coord, char hemisphere)
{
    if (!nmea_coord || strlen(nmea_coord) == 0)
        return 0;

    double raw = atof(nmea_coord);
    int degrees = static_cast<int>(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    double dec_deg = degrees + (minutes / 60.0);

    if (hemisphere == 'S' || hemisphere == 'W')
    {
        dec_deg = -dec_deg;
    }

    // Scale and convert to integer to save memory (Degrees * 10,000,000)
    return static_cast<int32_t>(dec_deg * 10000000.0);
}

// --- NMEA Sentence Parser ---
void parseNMEALine(char *line, GPSData &gps)
{
    // Reject corrupted UART strings immediately
    if (!validateNMEAChecksum(line))
        return;

    // Custom tokenizer that preserves empty fields (e.g., ",,")
    char *tokens[25];
    int token_count = 0;
    char *ptr = line;

    while (ptr != nullptr && token_count < 25)
    {
        tokens[token_count++] = ptr;
        char *comma = strchr(ptr, ',');
        if (comma)
        {
            *comma = '\0';   // Replace comma with null terminator
            ptr = comma + 1; // Move to the next token
        }
        else
        {
            // Terminate at the checksum asterisk or newline
            char *star = strpbrk(ptr, "*\r\n");
            if (star)
                *star = '\0';
            ptr = nullptr;
        }
    }

    if (token_count == 0)
        return;

    // 1. Parse $GPRMC
    if (strcmp(tokens[0], "$GPRMC") == 0 && token_count >= 10)
    {
        gps.valid = (strlen(tokens[2]) > 0 && tokens[2][0] == 'A');

        if (gps.valid)
        {
            // UTC Time
            if (strlen(tokens[1]) >= 6)
            {
                gps.hour = (tokens[1][0] - '0') * 10 + (tokens[1][1] - '0');
                gps.minute = (tokens[1][2] - '0') * 10 + (tokens[1][3] - '0');
                gps.second = (tokens[1][4] - '0') * 10 + (tokens[1][5] - '0');
            }

            // Coordinates
            if (strlen(tokens[3]) > 0 && strlen(tokens[4]) > 0)
            {
                gps.latitude = nmeaToScaledDegrees(tokens[3], tokens[4][0]);
            }
            if (strlen(tokens[5]) > 0 && strlen(tokens[6]) > 0)
            {
                gps.longitude = nmeaToScaledDegrees(tokens[5], tokens[6][0]);
            }

            // Speed (Convert knots to km/h, scale by 100)
            if (strlen(tokens[7]) > 0)
            {
                gps.speed_kmh = static_cast<uint16_t>(atof(tokens[7]) * 1.852 * 100.0);
            }
            else
            {
                gps.speed_kmh = 0;
            }

            // Heading (Scale by 100)
            if (strlen(tokens[8]) > 0)
            {
                gps.heading_deg = static_cast<uint16_t>(atof(tokens[8]) * 100.0);
            }
            else
            {
                gps.heading_deg = 0; // Default to 0 when standing still
            }

            // Date (Store as years since 2000 for bitfield optimization)
            if (strlen(tokens[9]) >= 6)
            {
                gps.day = (tokens[9][0] - '0') * 10 + (tokens[9][1] - '0');
                gps.month = (tokens[9][2] - '0') * 10 + (tokens[9][3] - '0');
                gps.year = ((tokens[9][4] - '0') * 10 + (tokens[9][5] - '0'));
            }
        }
    }
    // 2. Parse $GPGGA
    else if (strcmp(tokens[0], "$GPGGA") == 0 && token_count >= 10)
    {
        if (strlen(tokens[6]) > 0)
        {
            int fix_quality = atoi(tokens[6]);
            if (fix_quality > 0)
            {
                gps.valid = true;
                if (strlen(tokens[7]) > 0)
                    gps.satellites = atoi(tokens[7]);
                if (strlen(tokens[9]) > 0)
                    gps.altitude_m = static_cast<int16_t>(atof(tokens[9]));
            }
        }
    }
}

GPS::GPS(uart_inst_t *uart, uint baudrate, uint tx, uint rx) : m_uart(uart),
                                                               m_baudrate(baudrate),
                                                               m_tx(tx),
                                                               m_rx(rx),
                                                               m_buffer_index(0)
{
    memset(&m_data, 0, sizeof(GPSData)); // Zero out the struct memory

    uart_init(m_uart, m_baudrate);

    // Set the GPIO pin functions to UART
    gpio_set_function(m_tx, GPIO_FUNC_UART);
    gpio_set_function(m_rx, GPIO_FUNC_UART);

    // Default UART format: 8 data bits, 1 stop bit, no parity
    uart_set_format(m_uart, 8, 1, UART_PARITY_NONE);
}

float GPS::get_speed_kmh() const
{
    return m_data.speed_kmh / 100.0f;
}
uint8_t GPS::get_sat_counter() const
{
    return m_data.satellites;
}
int16_t GPS::get_altitude() const
{
    return m_data.altitude_m;
}
double GPS::get_latitude() const
{
    return m_data.latitude / 10000000.0;
}
double GPS::get_longitude() const
{
    return m_data.longitude / 10000000.0;
}
bool GPS::is_valid() const
{
    return m_data.valid;
}
float GPS::get_direction() const
{
    return m_data.heading_deg / 100.0f;
}

datetime_t GPS::get_datetime() const
{
    datetime_t dt;

    dt.year = m_data.year + 2000;
    dt.month = m_data.month;
    dt.day = m_data.day;
    dt.hour = m_data.hour;
    dt.min = m_data.minute;
    dt.sec = m_data.second;
    dt.dotw = 0; // dont care
    // // Calculate the Day of the Week using Sakamoto's algorithm.
    // int y = dt.year;
    // int m = dt.month;
    // static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    // y -= m < 3; // Subtract 1 from year if month is Jan or Feb
    // dt.dotw = (y + y/4 - y/100 + y/400 + t[m-1] + dt.day) % 7;
    return dt;
}

const GPSData *GPS::get_raw_value() const
{
    return &m_data;
}

bool GPS::update()
{
    // Read all available bytes in the hardware FIFO
    bool have_change = false;
    while (uart_is_readable(m_uart))
    {
        char c = uart_getc(m_uart);

        if (c == '\n')
        {
            m_buffer[m_buffer_index] = '\0';
            parseNMEALine(m_buffer, m_data);
            have_change = true;
            m_buffer_index = 0;
        }
        else if (c != '\r' && m_buffer_index < sizeof(m_buffer) - 1)
        {
            m_buffer[m_buffer_index++] = c;
        }
    }
    return have_change;
}
