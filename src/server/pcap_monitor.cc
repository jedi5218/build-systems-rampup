#include "pcap_monitor.h"

#include <cstring>

#include <common/utils.h>
#include "packet_structs.h"

void PCapThread::run()
{
    pcap_t *handle;                                                              /* Session handle */
    char *dev;                                                                   /* The device to sniff on */
    char errbuf[PCAP_ERRBUF_SIZE];                                               /* Error string */
    struct bpf_program fp;                                                       /* The compiled filter */
    char filter_exp[] = "tcp[tcpflags] & (tcp-syn|tcp-ack) = (tcp-syn|tcp-ack)"; /* The filter expression */
    bpf_u_int32 mask;                                                            /* Our netmask */
    bpf_u_int32 net;                                                             /* Our IP */
    struct pcap_pkthdr header;                                                   /* The header that pcap gives us */
    const u_char *packet;                                                        /* The actual packet */

    pcap_if_t *devs;
    handle_posix_error(pcap_findalldevs(&devs, errbuf), "Default device lookup");
    dev = devs->name;

    if (dev == NULL)
    {
        fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
        return;
    }
    /* Find the properties for the device */
    if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1)
    {
        fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
        net = 0;
        mask = 0;
    }
    /* Open the session in promiscuous mode */
    handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL)
    {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
        exit(1);
    }
    /* Compile and apply the filter */
    if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1)
    {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
        exit(1);
    }
    if (pcap_setfilter(handle, &fp) == -1)
    {
        fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
        exit(1);
    }

    printf("Started listening on the device %s\n", dev);

    pcap_loop(handle, 0, PCapThread::pcap_callback, reinterpret_cast<u_char *>(this));

    /* And close the session */
    pcap_close(handle);
    return;
}

void PCapThread::pcap_callback(u_char *user, const pcap_pkthdr *h, const u_char *bytes)
{
    PCapThread *obj = reinterpret_cast<PCapThread *>(user);
    obj->process_packet(h, bytes);
}

void PCapThread::process_packet(const pcap_pkthdr *h, const u_char *bytes)
{
    static int count = 1; /* packet counter */

    const struct sniff_ethernet *ethernet; /* The ethernet header [1] */
    const struct sniff_ip *ip;             /* The IP header */
    const struct sniff_tcp *tcp;           /* The TCP header */
    const char *payload;                   /* Packet payload */

    int size_ip;
    int size_tcp;
    int size_payload;

    printf("\nPacket number %d:\n", count);
    count++;

    ethernet = (struct sniff_ethernet *)(bytes);

    ip = (struct sniff_ip *)(bytes + SIZE_ETHERNET);
    size_ip = IP_HL(ip) * 4;
    if (size_ip < 20)
    {
        printf("   * Invalid IP header length: %u bytes\n", size_ip);
        return;
    }

    printf("       From: %s\n", inet_ntoa(ip->ip_src));
    printf("         To: %s\n", inet_ntoa(ip->ip_dst));

    switch (ip->ip_p)
    {
    case IPPROTO_TCP:
        printf("   Protocol: TCP\n");

        tcp = (struct sniff_tcp *)(bytes + SIZE_ETHERNET + size_ip);
        size_tcp = TH_OFF(tcp) * 4;
        if (size_tcp < 20)
        {
            printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
            return;
        }

        printf("   Src port: %d\n", ntohs(tcp->th_sport));
        printf("   Dst port: %d\n", ntohs(tcp->th_dport));

        tcp_deque.add_packet(ip->ip_src, ip->ip_dst, ntohs(tcp->th_sport), ntohs(tcp->th_dport));
        break;
    case IPPROTO_UDP:
        printf("   Protocol: UDP\n");
        return;
    case IPPROTO_ICMP:
        printf("   Protocol: ICMP\n");
        return;
    case IPPROTO_IP:
        printf("   Protocol: IP\n");
        return;
    default:
        printf("   Protocol: unknown\n");
        return;
    }
}

std::string ConnectionDeque::compile_report()
{
    std::lock_guard<std::mutex> guard(queue_mutex);
    char line_buffer[256] = {0};
    std::string retval;
    retval += "\nSource\tSource port\tDest.\tDest. port\n";
    for (const Connection &connection : connections)
    {
        sprintf(line_buffer, "%s\t%d\t%s\t%d\n\0",
                inet_ntoa(connection.src),
                connection.src_port,
                inet_ntoa(connection.dest),
                connection.dest_port);
        retval += line_buffer;
    }
    return retval;
}
void ConnectionDeque::add_packet(in_addr src, in_addr dest, u_char src_port, u_char dest_port)
{
    ConnectionDeque::Connection conn({src, dest, src_port, dest_port});
    if (!connection_set.count(conn))
    {
        std::lock_guard<std::mutex> guard(queue_mutex);
        connections.push_front(conn);
        connection_set.insert(conn);
        if (connections.size() > 100)
        {
            connection_set.erase(connections.back());
            connections.pop_back();
        }
    }
}
bool ConnectionDeque::Connection::operator==(const Connection &c) const
{
    return c.src.s_addr == src.s_addr and
           c.dest.s_addr == dest.s_addr and
           c.src_port == src_port and
           c.dest_port == dest_port;
}

inline void hash_combine(std::size_t &base, std::size_t add)
{
    base ^= add + 0x9e3779b9 + (base << 6) + (base >> 2);
}

std::size_t ConnectionDeque::Connection::hash::operator()(
    ConnectionDeque::Connection const &con) const noexcept
{
    size_t hash = 0;
    hash_combine(hash, std::hash<in_addr_t>{}(con.src.s_addr));
    hash_combine(hash, std::hash<in_addr_t>{}(con.dest.s_addr));
    hash_combine(hash, std::hash<ushort>{}(con.src_port));
    hash_combine(hash, std::hash<ushort>{}(con.dest_port));
    return hash;
}