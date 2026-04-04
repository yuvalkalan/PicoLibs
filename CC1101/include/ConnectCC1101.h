#pragma once
#include "CC1101.h"
#include <unordered_map>
#include <algorithm>
#include <vector>

#define TCP_FLAG_SYN 0b00000001       // syn flag bit
#define TCP_FLAG_ACK 0b00000010       // ack flag bit
#define TCP_FLAG_RSSI_LOW 0b00000100  // rssi low flag bit
#define TCP_FLAG_RSSI_HIGH 0b00001000 // rssi high flag bit
#define TCP_FLAG_START 0b00010000     // start of message flag bit

#define MAX_MSG_SIZE 1024 // make sure this is even!
#define TCP_RTO 100       // retransmission timeout in ms
#define TCP_MAX_RETRIES 5
#define TRACKER_T uint16_t

struct __attribute__((packed)) TCPPacketHeader : public PacketHeader
{
    uint8_t flags = 0;
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

uint16_t generate_random_number()
{
    // Simple random number generator using the current time as a seed
    return (uint16_t)(to_ms_since_boot(get_absolute_time()) & 0xFFFF);
}

class ConnectCC1101 : public CC1101
{

private:
    TRACKER_T m_ack;
    TRACKER_T m_syn;
    uint8_t m_tx_power_dbm;
    uint8_t m_rx_addr;
    uint16_t m_bytes_received = 0;
    uint16_t m_msg_length = 0;
    std::unordered_map<TRACKER_T, TCPPacketHandler> sending_packets;
    std::unordered_map<TRACKER_T, TCPPacket> received_packets;

private:
    void clear_rx_data();

public:
    void send(Msg &msg);
    bool receive(Msg &msg, uint32_t timeout_ms);
    void update();
    bool connect(uint8_t rx_addr, uint32_t timeout_ms);
    bool accept(uint32_t timeout_ms);

public:
    ConnectCC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address);
};

ConnectCC1101::ConnectCC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address) : CC1101(freq, mode, channel, address)
{
}

void ConnectCC1101::send(Msg &msg)
{
    // split msg to packets
    // first send msg length
    uint16_t total_length = msg.length + sizeof(msg.length);
    uint16_t bytes_sent = 0;
    printf("Sending message of length %d bytes\n", total_length);
    while (bytes_sent < total_length)
    {
        TCPPacketHandler handler;
        handler.packet.header.ack = m_ack;
        handler.packet.header.syn = m_syn++;
        handler.packet.header.flags = !bytes_sent ? TCP_FLAG_START : 0; // start flag for the first packet, data flag for subsequent packets
        handler.packet.header.rx_addr = m_rx_addr;                      // set receiver address
        uint16_t bytes_to_send = std::min((uint16_t)(CC1101_MAX_PACKET_LENGTH - sizeof(TCPPacketHeader)), (uint16_t)(total_length - bytes_sent));
        handler.packet.header.length = sizeof(TCPPacketHeader) + bytes_to_send;
        memcpy(handler.packet.payload, (uint8_t *)&msg + bytes_sent, bytes_to_send);

        // send_packet(m_rx_addr, (uint8_t *)&handler.packet, sizeof(PacketHeader) + bytes_to_send);
        sending_packets[handler.packet.header.syn] = handler;
        bytes_sent += bytes_to_send;
    }
    // // print packets in sending queue
    // printf("Packets in sending queue:\n");
    // for (const auto &entry : sending_packets)
    // {
    //     const TCPPacketHandler &handler = entry.second;
    //     printf("Packet SYN: %d, ACK: %d, Flags: %02X, Length: %d (%d) -> ", handler.packet.header.syn, handler.packet.header.ack, handler.packet.header.flags, handler.packet.header.length, handler.packet.header.length - sizeof(TCPPacketHeader), (char *)handler.packet.payload);
    //     // print payload as bytes and chars
    //     for (int i = 0; i < handler.packet.header.length - sizeof(TCPPacketHeader); i++)
    //     {
    //         printf("%02X ", handler.packet.payload[i]);
    //     }
    //     printf("(");
    //     for (int i = 0; i < handler.packet.header.length - sizeof(TCPPacketHeader); i++)
    //     {
    //         printf("%c ", handler.packet.payload[i]);
    //     }
    //     printf(")\n");
    //     printf("\n");
    // }
}

void ConnectCC1101::clear_rx_data()
{
    m_bytes_received = 0;
    m_msg_length = 0;
    received_packets.clear();
}

bool ConnectCC1101::receive(Msg &msg, uint32_t timeout_ms)
{
    // wait for packets and reassemble msg
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    while ((!m_msg_length || m_bytes_received < m_msg_length) && to_ms_since_boot(get_absolute_time()) - start_time < timeout_ms)
    {
        update();
        printf("Received %d/%d bytes\n", m_bytes_received, m_msg_length);
    }
    if (!m_msg_length || m_bytes_received < m_msg_length)
    {
        printf("Message receive timed out\n");
        clear_rx_data();
        return false;
    }
    // reassemble msg

    std::vector<TCPPacket> sorted_packets; // create a vector of received packets for sorting
    for (auto &entry : received_packets)
    {
        sorted_packets.push_back(entry.second);
    }

    std::sort(sorted_packets.begin(), sorted_packets.end(), [](const TCPPacket &a, const TCPPacket &b)
              { return (int16_t)(a.header.syn - b.header.syn) < 0; }); // sort packets by syn number, taking into account wrap-around

    // copy sorted packets to msg buffer
    uint16_t bytes_received = 0;
    for (const auto &packet : sorted_packets)
    {
        uint16_t payload_len = packet.header.length - sizeof(TCPPacketHeader);
        uint16_t bytes_to_copy = std::min(payload_len, (uint16_t)(m_msg_length - bytes_received));

        memcpy((uint8_t *)&msg + bytes_received, packet.payload, bytes_to_copy);
        bytes_received += bytes_to_copy;
        if (bytes_received >= m_msg_length)
            break;
    }

    clear_rx_data();
    return true;
}
void ConnectCC1101::update()
{
    // check for received packets, wait for a short time if no packets are received, and resend pending packets if their RTO has expired
    auto start_time = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - start_time < 10)
        while (packet_available())
        {
            printf("got packet!\n");
            uint8_t sender, lqi;
            int8_t rssi_dbm;
            TCPPacket packet;

            if (get_payload((Packet &)packet, rssi_dbm, lqi))
            {
                if (packet.header.flags & TCP_FLAG_START) // if this is the first packet of a message, extract the total message length
                {
                    m_msg_length = (packet.payload[0] | (packet.payload[1] << 8)) + sizeof(uint16_t); // assuming little-endian encoding of length, add 2 bytes for the length field itself
                    printf("Start of new message detected, total length: %d bytes\n", m_msg_length);
                }
                // check if it's an ack for a sent packet
                if (packet.header.flags & TCP_FLAG_ACK)
                {
                    printf("Received ACK for SYN %d\n", packet.header.ack);
                    // check if this packet exists in sending_packets
                    if (sending_packets.find(packet.header.ack) != sending_packets.end())
                    {
                        printf("ACK matches a sent packet, removing from sending queue\n");
                        sending_packets.erase(packet.header.ack);
                    }
                    else
                    {
                        printf("ACK does not match any sent packet, ignoring\n");
                    }
                }
                else
                {
                    // send ack
                    TCPPacket ack_packet;
                    ack_packet.header.rx_addr = packet.header.tx_addr; // reply to sender
                    ack_packet.header.ack = packet.header.syn;
                    ack_packet.header.syn = m_syn++;
                    ack_packet.header.flags = TCP_FLAG_ACK;                      // ack packet
                    ack_packet.header.length = sizeof(TCPPacketHeader);          // no payload
                    sending_packets[ack_packet.header.syn] = {ack_packet, 0, 0}; // add ack packet to sending queue for reliability
                    // send_packet((Packet &)ack_packet);

                    // add packet to received_packets
                    // check for duplicates
                    if (received_packets.find(packet.header.syn) != received_packets.end())
                    {
                        printf("Duplicate packet with syn %d received, ignoring\n", packet.header.syn);
                        continue;
                    }
                    m_bytes_received += packet.header.length - sizeof(TCPPacketHeader);
                    received_packets[packet.header.syn] = packet;
                }
            }
        }
    // send pending packets
    for (auto it = sending_packets.begin(); it != sending_packets.end();)
    {
        if (to_ms_since_boot(get_absolute_time()) - it->second.timestamp_ms > TCP_RTO)
        {
            if (it->second.retries >= TCP_MAX_RETRIES)
            {
                printf("Packet with syn %d failed to send after %d retries\n", it->second.packet.header.syn, TCP_MAX_RETRIES);
                it = sending_packets.erase(it);
            }
            else
            {
                printf("sending packet (%d) with syn %d, attempt %d\n", it->second.packet.header.length, it->second.packet.header.syn, it->second.retries + 1);
                send_packet((Packet &)(it->second.packet));
                if (it->second.packet.header.flags & TCP_FLAG_ACK)
                {
                    it = sending_packets.erase(it);
                    continue;
                }
                it->second.timestamp_ms = to_ms_since_boot(get_absolute_time());
                it->second.retries++;
                ++it;
                break;
                // sleep_ms(10); // small delay to avoid flooding the channel with retransmissions
            }
        }
        else
        {
            ++it;
        }
    }
}

bool ConnectCC1101::connect(uint8_t rx_addr, uint32_t timeout_ms)
{
    m_rx_addr = rx_addr;
    m_syn = generate_random_number();
    m_bytes_received = 0;

    // 1. הכנת פקטת SYN
    TCPPacket syn_packet;
    syn_packet.header.length = sizeof(TCPPacketHeader);
    syn_packet.header.rx_addr = m_rx_addr;
    syn_packet.header.tx_addr = m_address; // מוגדר במחלקת האב CC1101

    syn_packet.header.syn = m_syn;
    syn_packet.header.ack = 0;
    syn_packet.header.flags = TCP_FLAG_SYN;

    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    bool syn_ack_received = false;

    // שליחת פקטת ה-SYN על ידי קאסטינג למבנה הבסיס
    send_packet((Packet &)syn_packet);

    // 2. המתנה לפקטת SYN-ACK
    while (to_ms_since_boot(get_absolute_time()) - start_time < timeout_ms)
    {
        if (packet_available())
        {
            TCPPacket recv_packet;
            int8_t rssi_dbm;
            uint8_t lqi;

            if (get_payload((Packet &)recv_packet, rssi_dbm, lqi))
            {
                // וידוא שהפקטה מספיק גדולה כדי להכיל Header של TCP
                if (recv_packet.header.length >= sizeof(TCPPacketHeader))
                {
                    // וידוא שמדובר ב-SYN-ACK ושהוא מאשר את ה-SYN שלנו
                    if ((recv_packet.header.flags & (TCP_FLAG_SYN | TCP_FLAG_ACK)) == (TCP_FLAG_SYN | TCP_FLAG_ACK))
                    {
                        if (recv_packet.header.ack == m_syn)
                        {
                            m_ack = recv_packet.header.syn; // שמירת ה-Sequence של השרת
                            syn_ack_received = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!syn_ack_received)
    {
        printf("Connection failed: SYN-ACK timeout\n");
        return false;
    }

    // 3. שליחת ACK סופי לשרת
    TCPPacket ack_packet;
    ack_packet.header.length = sizeof(TCPPacketHeader);
    ack_packet.header.rx_addr = m_rx_addr;
    ack_packet.header.tx_addr = m_address;

    ack_packet.header.syn = m_syn++; // ניתן להשתמש ב-syn הבא שלנו כ-ack number
    ack_packet.header.ack = m_ack;
    ack_packet.header.flags = TCP_FLAG_ACK;

    send_packet((Packet &)ack_packet);

    printf("Connected successfully to address: 0x%02X\n", m_rx_addr);
    return true;
}
bool ConnectCC1101::accept(uint32_t timeout_ms)
{
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    bool syn_received = false;

    // 1. המתנה לפקטת SYN
    while (to_ms_since_boot(get_absolute_time()) - start_time < timeout_ms)
    {
        if (packet_available())
        {
            TCPPacket recv_packet;
            int8_t rssi_dbm;
            uint8_t lqi;

            if (get_payload((Packet &)recv_packet, rssi_dbm, lqi))
            {
                if (recv_packet.header.length >= sizeof(TCPPacketHeader))
                {
                    // בדיקה שזו פקטת SYN טהורה
                    if ((recv_packet.header.flags & TCP_FLAG_SYN) && !(recv_packet.header.flags & TCP_FLAG_ACK))
                    {
                        m_rx_addr = recv_packet.header.tx_addr; // השולח הוא כעת יעד התקשורת שלנו
                        m_ack = recv_packet.header.syn;
                        syn_received = true;
                        break;
                    }
                }
            }
        }
    }

    if (!syn_received)
    {
        return false; // פסק זמן - לא התקבלה בקשת התחברות
    }

    // 2. שליחת פקטת SYN-ACK
    m_syn = generate_random_number();

    TCPPacket syn_ack_packet;
    syn_ack_packet.header.length = sizeof(TCPPacketHeader);
    syn_ack_packet.header.rx_addr = m_rx_addr; // שולחים חזרה ללקוח
    syn_ack_packet.header.tx_addr = m_address;

    syn_ack_packet.header.syn = m_syn;
    syn_ack_packet.header.ack = m_ack;
    syn_ack_packet.header.flags = TCP_FLAG_SYN | TCP_FLAG_ACK;

    send_packet((Packet &)syn_ack_packet);

    // 3. המתנה ל-ACK סופי מהלקוח
    uint32_t ack_start_time = to_ms_since_boot(get_absolute_time());

    while (to_ms_since_boot(get_absolute_time()) - ack_start_time < (TCP_RTO * TCP_MAX_RETRIES))
    {
        if (packet_available())
        {
            TCPPacket ack_packet;
            int8_t rssi_dbm;
            uint8_t lqi;

            if (get_payload((Packet &)ack_packet, rssi_dbm, lqi))
            {
                if (ack_packet.header.length >= sizeof(TCPPacketHeader))
                {
                    // בדיקה שזהו ACK סופי המאשר את ה-SYN של השרת
                    if ((ack_packet.header.flags & TCP_FLAG_ACK) && !(ack_packet.header.flags & TCP_FLAG_SYN))
                    {
                        if (ack_packet.header.ack == m_syn)
                        {
                            printf("Connection established. Client: 0x%02X\n", m_rx_addr);
                            return true;
                        }
                    }
                }
            }
        }
    }
    m_rx_addr = 0; // איפוס כתובת הלקוח במידה והחיבור נכשל
    printf("Connection failed: Missing final ACK\n");
    return false;
}