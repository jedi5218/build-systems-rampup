#ifndef PCAP_MONITOR_H
#define PCAP_MONITOR_H

#include <pcap.h>
#include <unordered_set>
#include <deque>
#include <mutex>
#include <utility>
#include <netinet/in.h>
#include "thread.h"

class ConnectionDeque
{
public:
    std::string compile_report();
    void add_packet(in_addr src, in_addr dest, uint16_t src_port, uint16_t dest_port);

protected:
    std::mutex queue_mutex;
    struct Connection
    {
        in_addr src, dest;
        uint16_t src_port, dest_port;
        bool operator==(const Connection &c) const;
        struct hash
        {
            std::size_t operator()(Connection const &con) const noexcept;
        };
    };
    std::deque<Connection> connections;
    std::unordered_set<Connection, Connection::hash> connection_set;
};

class PCapThread : public Thread
{
public:
    ConnectionDeque &connections() { return tcp_deque; }

protected:
    void run() override;

private:
    static void pcap_callback(u_char *user, const pcap_pkthdr *h, const u_char *bytes);
    void process_packet(const pcap_pkthdr *h, const u_char *bytes);
    ConnectionDeque tcp_deque;
};

#endif //PCAP_MONITOR_H
