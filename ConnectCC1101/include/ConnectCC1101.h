#pragma once
#include "CC1101.h"
#include <unordered_map>
#include <algorithm>
#include <vector>
#include "pico/rand.h"

#define MAX_MSG_SIZE 1024 // make sure this is even!
#define TCP_MAX_RETRIES 20
#define TCP_TRANSMIT_TIMEOUT_FACTOR 3
#define TCP_RTO_FACTOR TCP_TRANSMIT_TIMEOUT_FACTOR * 2 // retransmission timeout

struct __attribute__((packed)) TCPFlags
{
    bool syn : 1;       // syn flag bit
    bool ack : 1;       // ack flag bit
    bool rssi_low : 1;  // rssi low flag bit
    bool rssi_high : 1; // rssi high flag bit
    bool start : 1;     // start of message flag bit
    bool end : 1;       // end of connection flag bit
    bool reserved : 2;
};

struct __attribute__((packed)) TCPPacketHeader : public PacketHeader
{
    TCPFlags flags = {0};
    uint16_t ack = 0;
    uint16_t syn = 0;
};

struct __attribute__((packed)) TCPPacket
{
    TCPPacketHeader header;
    uint8_t payload[CC1101_FIFOBUFFER - sizeof(header)];
};

struct __attribute__((packed)) TCPPacketHandler
{
    TCPPacket packet;
    uint8_t retries = 0;
};

struct __attribute__((packed)) Msg
{
    uint16_t length;
    uint8_t data[MAX_MSG_SIZE - sizeof(length)] = {0};
};

struct PendingAck
{
    uint8_t addr;
    uint16_t syn;
};

class ConnectCC1101 : public CC1101
{

private: // synconize data
    volatile uint16_t m_ack;
    volatile uint16_t m_syn;
    volatile uint8_t m_tx_power_dbm;
    volatile uint8_t m_rx_addr = 0;
    volatile uint16_t m_msg_length = 0;

private: // tx-rx clocks
    volatile uint32_t m_tx_timeout_us;
    volatile absolute_time_t m_last_receive_us;
    volatile absolute_time_t m_last_transmit_us;

private: // packets data structures
    std::unordered_map<uint16_t, TCPPacketHandler> m_sending_packets;
    std::unordered_map<uint16_t, TCPPacket> m_received_packets;
    std::vector<PendingAck> m_pending_acks;

private:
    bool update_tx();
    bool update_rx();
    void clear_rx();
    bool can_transmit();
    void calibrate_tx_speed();
    uint16_t check_bytes_received();

public:
    void send(Msg &msg);
    bool receive(Msg &msg, uint32_t timeout_ms);
    bool update();
    bool connect(uint8_t rx_addr, uint32_t timeout_ms);
    bool accept(uint32_t timeout_ms);
    bool is_connected();
    bool is_idle();
    bool have_data();

public:
    ConnectCC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address);
};