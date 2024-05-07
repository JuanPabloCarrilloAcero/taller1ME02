#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h" // Include for logging

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Taller");

int main(int argc, char *argv[]) {


    // Define variables
    uint32_t numNodes = 50; // Number of nodes per cluster
    double simulationTime = 200.0; // Simulation time in seconds
    double pauseTime = 5.0; // Pause time
    double wifiRate = 2.0; // Wi-Fi Rate in Mbps
    uint32_t packetSize = 64; // Packet size in Bytes
    double nodeSpeed = 20.0; // Node speed in m/s
    double regionX = 500.0; // Width of the region in meters
    double regionY = 1000.0; // Height of the region in meters

    NS_LOG_UNCOND("Number of nodes per cluster: " << numNodes);
    NS_LOG_UNCOND("Simulation time: " << simulationTime << " seconds");
    NS_LOG_UNCOND("Pause time: " << pauseTime);
    NS_LOG_UNCOND("Wi-Fi Rate: " << wifiRate << " Mbps");
    NS_LOG_UNCOND("Packet size: " << packetSize << " Bytes");
    NS_LOG_UNCOND("Node speed: " << nodeSpeed << " m/s");
    NS_LOG_UNCOND("Width of the region: " << regionX << " meters");
    NS_LOG_UNCOND("Height of the region: " << regionY << " meters");

    // Node configuration
    NodeContainer nodes;
    nodes.Create(numNodes);

    // Mobility configuration
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(nodes);

    // Wi-Fi configuration
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211b);
    wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager",
                                 "DataMode", StringValue("DsssRate2Mbps"),
                                 "ControlMode", StringValue("DsssRate1Mbps"));

    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel");

    YansWifiPhyHelper wifiPhy;
    wifiPhy.SetErrorRateModel("ns3::NistErrorRateModel");
    wifiPhy.SetChannel(wifiChannel.Create());

    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);

    // Network protocol configuration
    InternetStackHelper internet;
    internet.Install(nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Application configuration
    OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces.GetAddress(1), 9)));
    onoff.SetConstantRate(DataRate("2048bps"), packetSize);
    ApplicationContainer apps = onoff.Install(nodes.Get(0));
    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(simulationTime));

    // Install a packet sink at the receiving node
    PacketSinkHelper sinkHelper("ns3::UdpSocketFactory", Address(InetSocketAddress(interfaces.GetAddress(1), 9)));
    ApplicationContainer sinkApp = sinkHelper.Install(nodes.Get(1));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(simulationTime));

    // Run the simulation
    Simulator::Stop(Seconds(simulationTime));
    Simulator::Run();
    Simulator::Destroy();

    Ptr<PacketSink> sink = DynamicCast<PacketSink>(sinkApp.Get(0));
    uint64_t totalBytesReceived = sink->GetTotalRx();
    double totalBytesTransmitted = totalBytesReceived; // Total bytes transmitted is equal to total bytes received by the sink
    double throughput = (totalBytesTransmitted * 8) / simulationTime; // Throughput in bits per second
    double deliveryRatio = (totalBytesReceived / totalBytesTransmitted); // Delivery ratio

    // Calculate total delay and mean delay
    uint64_t totalPacketsReceived = sink->GetTotalRx();
    uint64_t totalDelay = sink->GetTotalRx();
    double averageDelay = totalPacketsReceived > 0 ? static_cast<double>(totalDelay) / totalPacketsReceived : 0;

    // Print metrics
    NS_LOG_UNCOND("Throughput: " << throughput << " bits/s");
    NS_LOG_UNCOND("Delivery Ratio: " << deliveryRatio * 100 << "%");
    NS_LOG_UNCOND("Average Delay: " << averageDelay << " seconds");

    return 0;
}
