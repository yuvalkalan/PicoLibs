#include "ConnectCC1101.h"

uint16_t generate_random_number()
{
    // Simple random number generator using the current time as a seed
    return (uint16_t)(to_ms_since_boot(get_absolute_time()) & 0xFFFF);
}

ConnectCC1101::ConnectCC1101(uint8_t freq, uint8_t mode, uint8_t channel, uint8_t address) : CC1101(freq, mode, channel, address)
{
}

void ConnectCC1101::send(Msg &msg)
{
    // split msg to packets
    // first send msg length
    uint16_t total_length = msg.length + sizeof(msg.length);
    uint16_t bytes_sent = 0;
    // printf("Sending message of length %d bytes\n", total_length);
    while (bytes_sent < total_length)
    {
        TCPPacketHandler handler;
        handler.packet.header.ack = m_ack;
        handler.packet.header.syn = m_syn++;
        handler.packet.header.flags.start = !bytes_sent; // start flag for the first packet
        handler.packet.header.rx_addr = m_rx_addr;       // set receiver address
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

void ConnectCC1101::clear_rx()
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
        // printf("Received %d/%d bytes\n", m_bytes_received, m_msg_length);
    }
    if (!m_msg_length || m_bytes_received < m_msg_length)
    {
        printf("Message receive timed out\n");
        clear_rx();
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

    clear_rx();
    return true;
}

// void ConnectCC1101::update_tx(){

// }

void ConnectCC1101::update()
{
    // check for received packets, wait for a short time if no packets are received, and resend pending packets if their RTO has expired
    auto start_time = to_ms_since_boot(get_absolute_time());
    while (to_ms_since_boot(get_absolute_time()) - start_time < 10)
        while (packet_available())
        {
            // printf("got packet!\n");
            uint8_t sender, lqi;
            int8_t rssi_dbm;
            TCPPacket packet;

            if (get_payload((Packet &)packet, rssi_dbm, lqi))
            {
                if (packet.header.flags.start) // if this is the first packet of a message, extract the total message length
                {
                    m_msg_length = (packet.payload[0] | (packet.payload[1] << 8)) + sizeof(uint16_t); // assuming little-endian encoding of length, add 2 bytes for the length field itself
                    // printf("Start of new message detected, total length: %d bytes\n", m_msg_length);
                }
                // check if it's an ack for a sent packet
                if (packet.header.flags.ack)
                {
                    // printf("Received ACK for SYN %d\n", packet.header.ack);
                    // check if this packet exists in sending_packets
                    if (sending_packets.find(packet.header.ack) != sending_packets.end())
                    {
                        // printf("ACK matches a sent packet, removing from sending queue\n");
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
                    ack_packet.header.flags.ack = true;                          // ack packet
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
                // printf("sending packet (%d) with syn %d, attempt %d\n", it->second.packet.header.length, it->second.packet.header.syn, it->second.retries + 1);
                send_packet((Packet &)(it->second.packet));
                if (it->second.packet.header.flags.ack)
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
    // clear data
    m_syn = generate_random_number();
    m_rx_addr = 0;
    clear_rx();
    sending_packets.clear();

    // send SYN packet ----------------------------------------------------------------------------
    TCPPacket syn_packet;
    syn_packet.header.length = sizeof(TCPPacketHeader);
    syn_packet.header.rx_addr = rx_addr;
    syn_packet.header.syn = m_syn;
    syn_packet.header.ack = 0;
    syn_packet.header.flags.syn = true;
    send_packet((Packet &)syn_packet);

    // wait for SYN-ACK packet --------------------------------------------------------------------
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    bool syn_ack_received = false;
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
                    if (recv_packet.header.flags.syn && recv_packet.header.flags.ack && recv_packet.header.tx_addr == rx_addr)
                    {
                        if (recv_packet.header.ack == m_syn)
                        {
                            m_ack = recv_packet.header.syn;
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
        Logger::print(LogLevel::ERROR, "Connection failed: SYN-ACK timeout\n");
        return false;
    }
    // send ACK packet ----------------------------------------------------------------------------
    TCPPacket ack_packet;
    ack_packet.header.length = sizeof(TCPPacketHeader);
    ack_packet.header.rx_addr = rx_addr;
    ack_packet.header.syn = ++m_syn;
    ack_packet.header.ack = m_ack;
    ack_packet.header.flags.ack = true;
    send_packet((Packet &)ack_packet);

    m_rx_addr = rx_addr;
    Logger::print(LogLevel::TRACE, "Connected successfully to address: 0x%02X, syn=%d, ack=%d\n", m_rx_addr, m_syn, m_ack);
    return true;
}

bool ConnectCC1101::accept(uint32_t timeout_ms)
{
    // wait for syn packet ------------------------------------------------------------------------
    bool syn_received = false;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
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
                    if ((recv_packet.header.flags.syn) && !(recv_packet.header.flags.ack))
                    {
                        m_rx_addr = recv_packet.header.tx_addr;
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
        Logger::print(LogLevel::ERROR, "Connection failed: SYN timeout\n");
        m_rx_addr = 0;
        return false;
    }

    // send SYN-ACK packet ------------------------------------------------------------------------

    m_syn = generate_random_number();
    TCPPacket syn_ack_packet;
    syn_ack_packet.header.length = sizeof(TCPPacketHeader);
    syn_ack_packet.header.rx_addr = m_rx_addr; // שולחים חזרה ללקוח
    syn_ack_packet.header.syn = m_syn;
    syn_ack_packet.header.ack = m_ack;
    syn_ack_packet.header.flags.syn = true;
    syn_ack_packet.header.flags.ack = true;
    send_packet((Packet &)syn_ack_packet);

    // wait for ACK -------------------------------------------------------------------------------
    while (to_ms_since_boot(get_absolute_time()) - start_time < timeout_ms)
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
                    if ((ack_packet.header.flags.ack) && !(ack_packet.header.flags.syn))
                    {
                        if (ack_packet.header.ack == m_syn && ack_packet.header.tx_addr == m_rx_addr)
                        {
                            m_syn++;
                            Logger::print(LogLevel::DEBUG, "Connection established. Client: 0x%02X syn=%d, ack=%d\n", m_rx_addr, m_syn, m_ack);
                            return true;
                        }
                    }
                }
            }
        }
    }
    m_rx_addr = 0;
    Logger::print(LogLevel::ERROR, "Connection failed: ACK timeout\n");
    return false;
}

bool ConnectCC1101::is_connected()
{
    return m_rx_addr != 0;
}

bool ConnectCC1101::is_idle()
{
    return sending_packets.empty() && received_packets.empty();
}

bool ConnectCC1101::have_data()
{
    return !received_packets.empty();
}