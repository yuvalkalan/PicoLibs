#include "GPS.h"

// Initialize static instance pointer for IRQ routing
GPS *GPS::s_instance = nullptr;

// Helper function to convert a single hex character to its integer value
int hex2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}

bool validateNMEAChecksum(const char *line)
{
    if (line[0] != '$')
        return false;
    uint8_t calculated_checksum = 0;
    int i = 1;
    while (line[i] != '*' && line[i] != '\0')
    {
        calculated_checksum ^= line[i];
        i++;
    }
    if (line[i] == '*')
    {
        int provided_checksum = (hex2int(line[i + 1]) << 4) | hex2int(line[i + 2]);
        return calculated_checksum == provided_checksum;
    }
    return false;
}

int32_t nmeaToScaledDegrees(const char *nmea_coord, char hemisphere)
{
    if (!nmea_coord || strlen(nmea_coord) == 0)
        return 0;
    double raw = atof(nmea_coord);
    int degrees = static_cast<int>(raw / 100.0);
    double minutes = raw - (degrees * 100.0);
    double dec_deg = degrees + (minutes / 60.0);
    if (hemisphere == 'S' || hemisphere == 'W')
        dec_deg = -dec_deg;
    return static_cast<int32_t>(dec_deg * 10000000.0);
}

void parseNMEALine(char *line, GPSData &gps)
{
    if (!validateNMEAChecksum(line))
        return;
    char *tokens[25];
    int token_count = 0;
    char *ptr = line;

    while (ptr != nullptr && token_count < 25)
    {
        tokens[token_count++] = ptr;
        char *comma = strchr(ptr, ',');
        if (comma)
        {
            *comma = '\0';
            ptr = comma + 1;
        }
        else
        {
            char *star = strpbrk(ptr, "*\r\n");
            if (star)
                *star = '\0';
            ptr = nullptr;
        }
    }
    if (token_count == 0)
        return;

    if (strcmp(tokens[0], "$GPRMC") == 0 && token_count >= 10)
    {
        gps.valid = (strlen(tokens[2]) > 0 && tokens[2][0] == 'A');
        if (gps.valid)
        {
            if (strlen(tokens[1]) >= 6)
            {
                gps.hour = (tokens[1][0] - '0') * 10 + (tokens[1][1] - '0');
                gps.minute = (tokens[1][2] - '0') * 10 + (tokens[1][3] - '0');
                gps.second = (tokens[1][4] - '0') * 10 + (tokens[1][5] - '0');
            }
            if (strlen(tokens[3]) > 0 && strlen(tokens[4]) > 0)
                gps.latitude = nmeaToScaledDegrees(tokens[3], tokens[4][0]);

            if (strlen(tokens[5]) > 0 && strlen(tokens[6]) > 0)
                gps.longitude = nmeaToScaledDegrees(tokens[5], tokens[6][0]);

            if (strlen(tokens[7]) > 0)
                gps.speed_kmh = static_cast<uint16_t>(atof(tokens[7]) * 1.852 * 100.0);
            else
                gps.speed_kmh = 0;

            if (strlen(tokens[8]) > 0)
                gps.heading_deg = static_cast<uint16_t>(atof(tokens[8]) * 100.0);
            else
                gps.heading_deg = 0;

            if (strlen(tokens[9]) >= 6)
            {
                gps.day = (tokens[9][0] - '0') * 10 + (tokens[9][1] - '0');
                gps.month = (tokens[9][2] - '0') * 10 + (tokens[9][3] - '0');
                gps.year = ((tokens[9][4] - '0') * 10 + (tokens[9][5] - '0'));
            }
        }
    }
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
void GPS::on_uart_rx_isr()
{
    if (s_instance) s_instance->isr_handler();
}

void GPS::isr_handler()
{
    while (uart_is_readable(m_uart))
    {
        char c = uart_getc(m_uart);

        if (c == '\n')
        {
            m_isr_buffer[m_isr_index] = '\0'; // Null-terminate

            // Drop the completed line into the appropriate mailbox
            if (strncmp(m_isr_buffer, "$GPRMC", 6) == 0)
            {
                strncpy((char*)m_latest_rmc, m_isr_buffer, sizeof(m_latest_rmc));
                m_rmc_ready = true;
            }
            else if (strncmp(m_isr_buffer, "$GPGGA", 6) == 0)
            {
                strncpy((char*)m_latest_gga, m_isr_buffer, sizeof(m_latest_gga));
                m_gga_ready = true;
            }

            m_isr_index = 0; // Reset for the next incoming line
        }
        else if (c != '\r' && m_isr_index < sizeof(m_isr_buffer) - 1)
        {
            m_isr_buffer[m_isr_index++] = c;
        }
    }
}

GPS::GPS(uart_inst_t *uart, uint baudrate, uint tx, uint rx) : m_uart(uart),
                                                               m_baudrate(baudrate),
                                                               m_tx(tx),
                                                               m_rx(rx),
                                                               m_isr_index(0),
                                                               m_rmc_ready(false),
                                                               m_gga_ready(false)
{
    memset(&m_data, 0, sizeof(GPSData));
    s_instance = this; 

    uart_init(m_uart, m_baudrate);
    gpio_set_function(m_tx, GPIO_FUNC_UART);
    gpio_set_function(m_rx, GPIO_FUNC_UART);
    uart_set_format(m_uart, 8, 1, UART_PARITY_NONE);

    int UART_IRQ = (m_uart == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, on_uart_rx_isr);
    irq_set_enabled(UART_IRQ, true);
    uart_set_irq_enables(m_uart, true, false); 
}

GPS::~GPS()
{
    int UART_IRQ = (m_uart == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_enabled(UART_IRQ, false);
    uart_set_irq_enables(m_uart, false, false);
    s_instance = nullptr;
}

float GPS::get_speed_kmh() const { return m_data.speed_kmh / 100.0f; }
uint8_t GPS::get_sat_counter() const { return m_data.satellites; }
int16_t GPS::get_altitude() const { return m_data.altitude_m; }
double GPS::get_latitude() const { return m_data.latitude / 10000000.0; }
double GPS::get_longitude() const { return m_data.longitude / 10000000.0; }
bool GPS::is_valid() const { return m_data.valid; }
float GPS::get_direction() const { return m_data.heading_deg / 100.0f; }
const GPSData *GPS::get_raw_value() const { return &m_data; }

datetime_t GPS::get_datetime() const
{
    datetime_t dt;
    dt.year = m_data.year + 2000;
    dt.month = m_data.month;
    dt.day = m_data.day;
    dt.hour = m_data.hour;
    dt.min = m_data.minute;
    dt.sec = m_data.second;
    dt.dotw = 0;
    return dt;
}

bool GPS::update()
{
    bool have_change = false;
    int UART_IRQ = (m_uart == uart0) ? UART0_IRQ : UART1_IRQ;

    // Safely retrieve and parse the RMC mailbox
    if (m_rmc_ready)
    {
        // Disable interrupt briefly so the ISR doesn't overwrite while we copy
        irq_set_enabled(UART_IRQ, false);
        strncpy(m_buffer, (const char*)m_latest_rmc, sizeof(m_buffer));
        m_rmc_ready = false;
        irq_set_enabled(UART_IRQ, true);

        parseNMEALine(m_buffer, m_data);
        have_change = true;
    }

    // Safely retrieve and parse the GGA mailbox
    if (m_gga_ready)
    {
        irq_set_enabled(UART_IRQ, false);
        strncpy(m_buffer, (const char*)m_latest_gga, sizeof(m_buffer));
        m_gga_ready = false;
        irq_set_enabled(UART_IRQ, true);

        parseNMEALine(m_buffer, m_data);
        have_change = true;
    }

    return have_change;
}