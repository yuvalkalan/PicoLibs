#pragma once
#include "CC1101.h"
#include "Logger.h"
#include <unordered_map>
#include <algorithm>
#include <vector>

#define MAX_MSG_SIZE 1024 // make sure this is even!
#define TCP_RTO 100       // retransmission timeout in ms
#define TCP_MAX_RETRIES 5
#define TRACKER_T uint16_t
#define TRANSMIT_TIMEOUT_FACTOR 4

uint16_t generate_random_number();

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
    TRACKER_T ack = 0;
    TRACKER_T syn = 0;
};

struct __attribute__((packed)) TCPPacket
{
    TCPPacketHeader header;
    uint8_t payload[CC1101_FIFOBUFFER - sizeof(header)];
};

struct __attribute__((packed)) TCPPacketHandler
{
    TCPPacket packet;
    uint32_t timestamp_ms = 0;
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
    TRACKER_T syn;
};

class ConnectCC1101 : public CC1101
{

private:
    TRACKER_T m_ack;
    TRACKER_T m_syn;
    uint8_t m_tx_power_dbm;
    uint8_t m_rx_addr = 0;
    uint16_t m_bytes_received = 0;
    uint16_t m_msg_length = 0;
    std::unordered_map<TRACKER_T, TCPPacketHandler> sending_packets;
    std::unordered_map<TRACKER_T, TCPPacket> received_packets;
    std::vector<PendingAck> pending_acks;
    uint32_t m_tx_timeout_us;
    absolute_time_t m_last_receive_us;
    uint8_t m_packet_group_id = 0;
    uint8_t m_packet_group_next = 0;

private:
    void update_tx();
    void update_rx();
    void clear_rx();
    bool can_transmit();
    void calibrate_tx_speed();

public:
    void send(Msg &msg);
    bool receive(Msg &msg, uint32_t timeout_ms);
    void update();
    bool connect(uint8_t rx_addr, uint32_t timeout_ms);
    bool accept(uint32_t timeout_ms);
    bool is_connected();
    bool is_idle();
    bool have_data();

public:
    ConnectCC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address);
};