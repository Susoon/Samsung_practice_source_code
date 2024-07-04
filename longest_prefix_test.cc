#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/ipv4-static-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("LongestPrefixMatchExample");

NodeContainer nodes;

#define SERVER1 3
#define SERVER2 4

void LogRoutingTable(Ptr<Node> node)
{
    Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
    if (ipv4)
    {
        Ptr<Ipv4StaticRouting> routing = 0;
        Ipv4StaticRoutingHelper routingHelper;
        routing = routingHelper.GetStaticRouting(ipv4);
        if (routing)
        {
            NS_LOG_INFO("Node " << node->GetId() << " ip: ");
			for(int i = 1; i < ipv4->GetNInterfaces(); i++){
				for(int j = 0; j < ipv4->GetNAddresses(i); j++){
					Ipv4Address addr = ipv4->GetAddress(i, j).GetLocal();
					NS_LOG_INFO(addr);
				}
			}
            NS_LOG_INFO("Node " << node->GetId() << " routing table:");
			std::ostringstream oss;
            routing->PrintRoutingTable(Create<OutputStreamWrapper>(&oss));
			NS_LOG_INFO(oss.str());
        }
        else
        {
            NS_LOG_WARN("Static routing not found for node " << node->GetId());
        }
    }
    else
    {
        NS_LOG_WARN("IPv4 not found for node " << node->GetId());
    }
}

void PacketReceivedCallback(Ptr<const Packet> packet, const Address &addr1, const Address &addr2)
{
    NS_LOG_INFO("Packet received from " << InetSocketAddress::ConvertFrom(addr1).GetIpv4() << " port "
                                        << InetSocketAddress::ConvertFrom(addr1).GetPort()
                                        << " at " << InetSocketAddress::ConvertFrom(addr2).GetIpv4() << " port "
                                        << InetSocketAddress::ConvertFrom(addr2).GetPort());
    //NS_LOG_INFO("Packet received with size: " << packet->GetSize());

    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        LogRoutingTable(nodes.Get(i));
    }
}

void PacketReceivedCallbackWrapper1(Ptr<const Packet> packet, const Address &addr1, const Address &addr2)
{
    Ptr<Node> node = nodes.Get(SERVER2);
    NS_LOG_INFO("Server Node " << node->GetId() << " received packet");
	PacketReceivedCallback(packet, addr1, addr2);
}

void PacketReceivedCallbackWrapper2(Ptr<const Packet> packet, const Address &addr1, const Address &addr2)
{
    Ptr<Node> node = nodes.Get(SERVER2);
    NS_LOG_INFO("Server Node " << node->GetId() << " received packet");
	PacketReceivedCallback(packet, addr1, addr2);
}

std::string MakeTracePath(int node_id)
{
	std::ostringstream oss;
	oss << "/NodeList/" << node_id << "/ApplicationList/*/$ns3::UdpEchoServer/RxWithAddresses";
	std::string tracePath = oss.str();

	return tracePath;
}

int main(int argc, char *argv[])
{
    // Enable logging
    LogComponentEnable("LongestPrefixMatchExample", LOG_LEVEL_INFO);
    LogComponentEnable("Ipv4StaticRouting", LOG_LEVEL_INFO);

    // Create nodes
    nodes.Create(6);

    // Create point-to-point links
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    // Network topology
    // n0 -- n1 -- n2 -- n3
    //       |      |
    //      n4     n5

    NetDeviceContainer devices01 = pointToPoint.Install(nodes.Get(0), nodes.Get(1));
    NetDeviceContainer devices12 = pointToPoint.Install(nodes.Get(1), nodes.Get(2));
    NetDeviceContainer devices23 = pointToPoint.Install(nodes.Get(2), nodes.Get(3));
    NetDeviceContainer devices14 = pointToPoint.Install(nodes.Get(1), nodes.Get(4));
    NetDeviceContainer devices25 = pointToPoint.Install(nodes.Get(2), nodes.Get(5));

    // Install Internet stack
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses
    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces01 = address.Assign(devices01);
    address.SetBase("192.168.2.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces12 = address.Assign(devices12);
    address.SetBase("192.168.3.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces23 = address.Assign(devices23);
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces14 = address.Assign(devices14);
    address.SetBase("172.16.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces25 = address.Assign(devices25);

    // Add static routes
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> staticRouting;

    staticRouting = ipv4RoutingHelper.GetStaticRouting(nodes.Get(0)->GetObject<Ipv4>());
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.2.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.1.2"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.3.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.1.2"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.1.2"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("172.16.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.1.2"), 1);

    staticRouting = ipv4RoutingHelper.GetStaticRouting(nodes.Get(1)->GetObject<Ipv4>());
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.3.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.2.2"), 2);
    staticRouting->AddNetworkRouteTo(Ipv4Address("172.16.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.2.2"), 2);

    staticRouting = ipv4RoutingHelper.GetStaticRouting(nodes.Get(2)->GetObject<Ipv4>());
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.2.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.2.1"), 1);

    staticRouting = ipv4RoutingHelper.GetStaticRouting(nodes.Get(3)->GetObject<Ipv4>());
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.3.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.3.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("172.16.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("192.168.3.1"), 1);

    staticRouting = ipv4RoutingHelper.GetStaticRouting(nodes.Get(4)->GetObject<Ipv4>());
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.2.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("10.1.1.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.3.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("10.1.1.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("172.16.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("10.1.1.1"), 1);

    staticRouting = ipv4RoutingHelper.GetStaticRouting(nodes.Get(5)->GetObject<Ipv4>());
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("172.16.1.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("10.1.1.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("172.16.1.1"), 1);
    staticRouting->AddNetworkRouteTo(Ipv4Address("192.168.3.0"), Ipv4Mask("255.255.255.0"), Ipv4Address("172.16.1.1"), 1);

    // Log routing tables
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        LogRoutingTable(nodes.Get(i));
    }

    // Install applications
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(SERVER1));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoServerHelper echoServer2(10);
    ApplicationContainer serverApps2 = echoServer2.Install(nodes.Get(SERVER2));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient1(Ipv4Address("192.168.3.2"), 9);
    echoClient1.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient1.SetAttribute("PacketSize", UintegerValue(1024));

    UdpEchoClientHelper echoClient2(Ipv4Address("10.1.1.2"), 10);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(5));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps1 = echoClient1.Install(nodes.Get(0));
    clientApps1.Start(Seconds(2.0));
    clientApps1.Stop(Seconds(10.0));

    ApplicationContainer clientApps2 = echoClient2.Install(nodes.Get(5));
    clientApps2.Start(Seconds(2.5));
    clientApps2.Stop(Seconds(10.0));

    // Packet receive callback
	std::string tracePath = MakeTracePath(SERVER1);
    Config::ConnectWithoutContext(tracePath, MakeCallback(&PacketReceivedCallbackWrapper1));

	tracePath = MakeTracePath(SERVER2);
   	Config::ConnectWithoutContext(tracePath, MakeCallback(&PacketReceivedCallbackWrapper2));

    // Setup NetAnim
    AnimationInterface anim("longest-prefix-match.xml");
    anim.SetConstantPosition(nodes.Get(0), 30.0, 20.0);
    anim.SetConstantPosition(nodes.Get(1), 50.0, 20.0);
    anim.SetConstantPosition(nodes.Get(2), 70.0, 20.0);
    anim.SetConstantPosition(nodes.Get(3), 90.0, 20.0);
    anim.SetConstantPosition(nodes.Get(4), 50.0, 40.0);
    anim.SetConstantPosition(nodes.Get(5), 70.0, 40.0);

    // Run simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

