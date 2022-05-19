#include <CLI/CLI.hpp>

#include <ethminer/buildinfo.h>
#include <condition_variable>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

#include <libethcore/Farm.h>
#if ETH_ETHASHCL
#include <libethash-cl/CLMiner.h>
#endif
#if ETH_ETHASHCUDA
#include <libethash-cuda/CUDAMiner.h>
#endif
#if ETH_ETHASHCPU
#include <libethash-cpu/CPUMiner.h>
#endif
#include <libpoolprotocols/PoolManager.h>

#if API_CORE
#include <libapicore/ApiServer.h>
#include <regex>
#endif

#if defined(__linux__) || defined(__APPLE__)
#include <execinfo.h>
#elif defined(_WIN32)
#include <Windows.h>
#endif

#include <execinfo.h>
#include <condition_variable>

bool g_running = false;
bool g_exitOnError = false;  // Whether or not ethminer should exit on mining threads errors

condition_variable g_shouldstop;
boost::asio::io_service g_io_service;  // The IO service itself


int main(int argc, char** argv)
{
  std::map<std::string, DeviceDescriptor> m_DevicesCollection = {};
  FarmSettings m_FarmSettings;  // Operating settings for Farm
  PoolSettings m_PoolSettings;  // Operating settings for PoolManager
  CLSettings m_CLSettings;          // Operating settings for CL Miners
  CUSettings m_CUSettings;          // Operating settings for CUDA Miners
  CPSettings m_CPSettings;    // Operating settings for CPU Miners
  m_CPSettings.devices = {0U, 1U};

  #if ETH_ETHASHCL
    CLMiner::enumDevices(m_DevicesCollection);
#endif
#if ETH_ETHASHCUDA
    CUDAMiner::enumDevices(m_DevicesCollection);
#endif
#if ETH_ETHASHCPU
    CPUMiner::enumDevices(m_DevicesCollection);

    if (m_CPSettings.devices.size())
    {
        for (auto index : m_CPSettings.devices)
        {
            if (index < m_DevicesCollection.size())
            {
                auto it = m_DevicesCollection.begin();
                std::advance(it, index);
                it->second.subscriptionType = DeviceSubscriptionTypeEnum::Cpu;
            }
        }
    }
#endif

    // Can't proceed without any GPU
    if (m_DevicesCollection.empty())
        throw std::runtime_error("No usable mining devices found");

  m_FarmSettings.ergodicity = false;
  m_FarmSettings.hwMon = false;
  m_FarmSettings.noEval = false;
  m_FarmSettings.dagLoadMode;
  m_FarmSettings.tempStop = 80;
  m_FarmSettings.tempStart = 40;


#if ETH_ETHASHCUDA
  m_CUSettings.schedule = 0;
#endif

  new Farm(m_DevicesCollection, m_FarmSettings, m_CUSettings, m_CLSettings, m_CPSettings);

  Farm::f().start();

  WorkPackage m_currentWp;

  int block = 10000000;

  m_currentWp.job.clear();
  m_currentWp.header = h256();
  m_currentWp.epoch = block / 30000;
  m_currentWp.block = block;
  m_currentWp.header   = h256("1234567890123456789012345678901234567890123456789012345678901234");
  m_currentWp.seed = h256("123");
  m_currentWp.boundary = h256("42ff52d3522cffffffffffffffffffffffffffffffffffffffffffffffffffff");
  m_currentWp.exSizeBytes = 0;
  m_currentWp.startNonce = 0;

  std::cout << m_currentWp.header << std::endl;

  Farm::f().setWork(m_currentWp);

    sleep(1000);
}
