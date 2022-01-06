// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2016-2018, The Karbo developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2021 Conceal Network & Conceal Devs
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <Logging/LoggerManager.h>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "Common/PathTools.h"
#include "Common/SignalHandler.h"
#include "CryptoNoteCore/Core.h"
#include "CryptoNoteCore/CoreConfig.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteCore/Currency.h"
#include "CryptoNoteCore/MinerConfig.h"
#include "CryptoNoteProtocol/CryptoNoteProtocolHandler.h"
#include "DaemonCommandsHandler.h"
#include "Logging/ConsoleLogger.h"
#include "P2p/NetNode.h"
#include "P2p/NetNodeConfig.h"
#include "Rpc/RpcServer.h"
#include "Rpc/RpcServerConfig.h"
#include "crypto/hash.h"
#include "version.h"

#if defined(WIN32)
#include <crtdbg.h>
#endif

using common::JsonValue;
using namespace cn;
using namespace logging;

namespace po = boost::program_options;

bool command_line_preprocessor(const boost::program_options::variables_map& vm, LoggerRef& logger);

void print_genesis_tx_hex()
{
  logging::ConsoleLogger logger;
  cn::Transaction tx = cn::CurrencyBuilder(logger).generateGenesisTransaction();
  cn::BinaryArray txb = cn::toBinaryArray(tx);
  std::string tx_hex = common::toHex(txb);

  std::cout << "Insert this line into your coin configuration file as is: " << std::endl;
  std::cout << "const char GENESIS_COINBASE_TX_HEX[] = \"" << tx_hex << "\";" << std::endl;

  return;
}

// void print_genesis_tx_hex(const po::variables_map& vm) {
  // std::vector<cn::AccountPublicAddress> targets;
 //  auto genesis_block_reward_addresses = command_line::get_arg(vm, arg_genesis_block_reward_address);

//   logging::ConsoleLogger logger;
//   cn::CurrencyBuilder currencyBuilder(logger);

 //  cn::Currency currency = currencyBuilder.currency();

 //  for (const auto& address_string : genesis_block_reward_addresses) {
 //     cn::AccountPublicAddress address;
   //  if (!currency.parseAccountAddressString(address_string, address)) {
   //    std::cout << "Failed to parse address: " << address_string << std::endl;
   //    return;
  //   }
 //	//Print GENESIS_BLOCK_REWARD Mined Address
 //	std::cout << "Your SDN Pre-mined Address String is:  " << address_string << std::endl;
 //    targets.emplace_back(std::move(address));
//   }

 //  if (targets.empty()) {
 //    if (cn::parameters::GENESIS_BLOCK_REWARD > 0) {
 //      std::cout << "Error: genesis block reward addresses are not defined" << std::endl;
 //    } else {

 //	  cn::Transaction tx = cn::CurrencyBuilder(logger).generateGenesisTransaction();
 //	  cn::BinaryArray txb = cn::toBinaryArray(tx);
 //	  std::string tx_hex = common::toHex(txb);

// 	  std::cout << "Insert this line into your coin configuration file as is: " << std::endl;
//	  std::cout << "const char GENESIS_COINBASE_TX_HEX[] = \"" << tx_hex << "\";" << std::endl;
//	}
//   } else {
 //	cn::Transaction tx = cn::CurrencyBuilder(logger).generateGenesisTransaction(targets);
 //	cn::BinaryArray txb = cn::toBinaryArray(tx);
 //	std::string tx_hex = common::toHex(txb);

//	std::cout << "Modify this line into your concealX configuration file as is:  " << std::endl;
//	std::cout << "const char GENESIS_COINBASE_TX_HEX[] = \"" << tx_hex << "\";" << std::endl;
//   }
//   return;
// }

JsonValue buildLoggerConfiguration(Level level, const std::string& logfile)
{
  JsonValue loggerConfiguration(JsonValue::OBJECT);
  loggerConfiguration.insert("globalLevel", static_cast<int64_t>(level));

  JsonValue& cfgLoggers = loggerConfiguration.insert("loggers", JsonValue::ARRAY);

  JsonValue& fileLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  fileLogger.insert("type", "file");
  fileLogger.insert("filename", logfile);
  fileLogger.insert("level", static_cast<int64_t>(TRACE));

  JsonValue& consoleLogger = cfgLoggers.pushBack(JsonValue::OBJECT);
  consoleLogger.insert("type", "console");
  consoleLogger.insert("level", static_cast<int64_t>(TRACE));
  consoleLogger.insert("pattern", "%T %L ");

  return loggerConfiguration;
}

void renameDataDir(std::string directory)
{
  std::string concealXDir = directory;
  boost::filesystem::path concealXDirPath(concealXDir);
  if (boost::filesystem::exists(concealXDirPath))
  {
    return;
  }

  std::string dataDirPrefix =
      concealXDir.substr(0, concealXDir.size() + 1 - sizeof(CRYPTONOTE_NAME));
  boost::filesystem::path cediDirPath(dataDirPrefix + "BXC");

  if (boost::filesystem::exists(cediDirPath))
  {
    boost::filesystem::rename(cediDirPath, concealXDirPath);
  }
  else
  {
    boost::filesystem::path BcediDirPath(dataDirPrefix + "Bcedi");
    if (boost::filesystem::exists(boost::filesystem::path(BcediDirPath)))
    {
      boost::filesystem::rename(BcediDirPath, concealXDirPath);
    }
  }
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  LoggerManager logManager;
  LoggerRef logger(logManager, "daemon");

  try
  {
    po::options_description desc_cmd_only("Command line options");
    po::options_description desc_cmd_sett("Command line options and settings options");

    desc_cmd_sett.add_options()("enable-blockchain-indexes,i",
                                po::bool_switch()->default_value(false),
                                "Enable blockchain indexes");
    desc_cmd_sett.add_options()("enable-autosave,a", po::bool_switch()->default_value(false),
                                "Enable blockchain autosave every 720 blocks");

    command_line::add_arg(desc_cmd_only, command_line::arg_help);
    command_line::add_arg(desc_cmd_only, command_line::arg_version);
    command_line::add_arg(desc_cmd_only, command_line::arg_os_version);
    command_line::add_arg(desc_cmd_only, command_line::arg_data_dir);
    command_line::add_arg(desc_cmd_only, command_line::arg_config_file);
    command_line::add_arg(desc_cmd_sett, command_line::arg_set_fee_address);
    command_line::add_arg(desc_cmd_sett, command_line::arg_log_file);
    command_line::add_arg(desc_cmd_sett, command_line::arg_log_level);
    command_line::add_arg(desc_cmd_sett, command_line::arg_console);
    command_line::add_arg(desc_cmd_sett, command_line::arg_set_view_key);
    command_line::add_arg(desc_cmd_sett, command_line::arg_testnet_on);
    command_line::add_arg(desc_cmd_sett, command_line::arg_print_genesis_tx);
    // command_line::add_arg(desc_cmd_sett, arg_genesis_block_reward_address);

    RpcServerConfig::initOptions(desc_cmd_sett);
    CoreConfig::initOptions(desc_cmd_sett);
    NetNodeConfig::initOptions(desc_cmd_sett);
    MinerConfig::initOptions(desc_cmd_sett);

    po::options_description desc_options("Allowed options");
    desc_options.add(desc_cmd_only).add(desc_cmd_sett);

    po::variables_map vm;
    CoreConfig coreConfig;

    bool r = command_line::handle_error_helper(desc_options, [&]() {
      po::store(po::parse_command_line(argc, argv, desc_options), vm);

      coreConfig.init(vm);

      if (command_line::get_arg(vm, command_line::arg_help))
      {
        std::cout << cn::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL << ENDL;
        std::cout << desc_options << std::endl;
        return false;
      }

      if (command_line::get_arg(vm, command_line::arg_print_genesis_tx))
      {
        // print_genesis_tx_hex(vm);
        print_genesis_tx_hex();
        return false;
      }

      std::string data_dir = coreConfig.configFolder;
      std::string config = command_line::get_arg(vm, command_line::arg_config_file);

      boost::filesystem::path data_dir_path(data_dir);
      boost::filesystem::path config_path(config);
      if (!config_path.has_parent_path())
      {
        config_path = data_dir_path / config_path;
      }

      boost::system::error_code ec;
      if (boost::filesystem::exists(config_path, ec))
      {
        po::store(
            po::parse_config_file<char>(config_path.string<std::string>().c_str(), desc_cmd_sett),
            vm);
      }

      po::notify(vm);
      return true;
    });

    if (!r)
    {
      return 1;
    }

    NetNodeConfig netNodeConfig;
    netNodeConfig.init(vm);
    netNodeConfig.setTestnet(coreConfig.testnet);
    netNodeConfig.setConfigFolder(coreConfig.configFolder);
    MinerConfig minerConfig;
    minerConfig.init(vm);
    RpcServerConfig rpcConfig;
    rpcConfig.init(vm);

    renameDataDir(coreConfig.configFolder);

    auto modulePath = common::NativePathToGeneric(argv[0]);
    auto cfgLogFile =
        common::NativePathToGeneric(command_line::get_arg(vm, command_line::arg_log_file));

    if (cfgLogFile.empty())
    {
      cfgLogFile = common::ReplaceExtenstion(modulePath, ".log");
    }
    else
    {
      if (!common::HasParentPath(cfgLogFile))
      {
        cfgLogFile = common::CombinePath(common::GetPathDirectory(modulePath), cfgLogFile);
      }
    }

    Level cfgLogLevel = static_cast<Level>(static_cast<int>(logging::ERROR) +
                                           command_line::get_arg(vm, command_line::arg_log_level));

    // configure logging
    logManager.configure(buildLoggerConfiguration(cfgLogLevel, cfgLogFile));

    logger(INFO, BRIGHT_YELLOW) << "Conceal v" << PROJECT_VERSION_LONG;

    if (command_line_preprocessor(vm, logger))
    {
      return 0;
    }

    logger(INFO) << "Module folder: " << argv[0];

    logger(INFO) << "Blockchain and configuration folder: " << coreConfig.configFolder;

    if (coreConfig.testnet)
    {
      logger(INFO, MAGENTA) << "/!\\ Starting in testnet mode /!\\";
    }

    // create objects and link them
    cn::CurrencyBuilder currencyBuilder(logManager);
    currencyBuilder.testnet(coreConfig.testnet);

    try
    {
      currencyBuilder.currency();
    }
    catch (std::exception&)
    {
      std::cout << "GENESIS_COINBASE_TX_HEX constant has an incorrect value. Please launch: "
                << cn::CRYPTONOTE_NAME << "d --" << command_line::arg_print_genesis_tx.name;
      return 1;
    }

    cn::Currency currency = currencyBuilder.currency();
    cn::core ccore(currency, nullptr, logManager,
                           vm["enable-blockchain-indexes"].as<bool>(),
                           vm["enable-autosave"].as<bool>());

    if (!coreConfig.configFolderDefaulted)
    {
      if (!tools::directoryExists(coreConfig.configFolder))
      {
        throw std::runtime_error("Directory does not exist: " + coreConfig.configFolder);
      }
    }
    else
    {
      if (!tools::createDirectoriesIfNecessary(coreConfig.configFolder))
      {
        throw std::runtime_error("Can't create directory: " + coreConfig.configFolder);
      }
    }

    platform_system::Dispatcher dispatcher;

    cn::CryptoNoteProtocolHandler cprotocol(currency, dispatcher, ccore, nullptr,
                                                    logManager);
    cn::NodeServer p2psrv(dispatcher, cprotocol, logManager);
    cn::RpcServer rpcServer(dispatcher, logManager, ccore, p2psrv, cprotocol);

    cprotocol.set_p2p_endpoint(&p2psrv);
    ccore.set_cryptonote_protocol(&cprotocol);
    DaemonCommandsHandler dch(ccore, p2psrv, logManager);

    // initialize objects
    logger(INFO) << "Initializing p2p server...";
    if (!p2psrv.init(netNodeConfig))
    {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize p2p server.";
      return 1;
    }

    logger(INFO) << "P2p server initialized OK";

    // initialize core here
    logger(INFO) << "Initializing core...";
    if (!ccore.init(coreConfig, minerConfig, true))
    {
      logger(ERROR, BRIGHT_RED) << "Failed to initialize core";
      return 1;
    }

    logger(INFO) << "Core initialized OK";

    // start components
    if (!command_line::has_arg(vm, command_line::arg_console))
    {
      dch.start_handling();
    }

    logger(INFO) << "Starting core rpc server on address " << rpcConfig.getBindAddress();

    /* Set address for remote node fee */
    if (command_line::has_arg(vm, command_line::arg_set_fee_address))
    {
      std::string addr_str = command_line::get_arg(vm, command_line::arg_set_fee_address);
      if (!addr_str.empty())
      {
        AccountPublicAddress acc = boost::value_initialized<AccountPublicAddress>();
        if (!currency.parseAccountAddressString(addr_str, acc))
        {
          logger(ERROR, BRIGHT_RED) << "Bad fee address: " << addr_str;
          return 1;
        }
        rpcServer.setFeeAddress(addr_str, acc);
        logger(INFO, BRIGHT_YELLOW) << "Remote node fee address set: " << addr_str;
      }
    }

    /* This sets the view-key so we can confirm that
       the fee is part of the transaction blob */
    if (command_line::has_arg(vm, command_line::arg_set_view_key))
    {
      std::string vk_str = command_line::get_arg(vm, command_line::arg_set_view_key);
      if (!vk_str.empty())
      {
        rpcServer.setViewKey(vk_str);
        logger(INFO, BRIGHT_YELLOW) << "Secret view key set: " << vk_str;
      }
    }

    rpcServer.start(rpcConfig.bindIp, rpcConfig.bindPort);
    logger(INFO) << "Core rpc server started ok";

    tools::SignalHandler::install([&dch, &p2psrv] {
      dch.stop_handling();
      p2psrv.sendStopSignal();
    });

    logger(INFO) << "Starting p2p net loop...";
    p2psrv.run();
    logger(INFO) << "p2p net loop stopped";

    dch.stop_handling();

    // stop components
    logger(INFO) << "Stopping core rpc server...";
    rpcServer.stop();

    // deinitialize components
    logger(INFO) << "Deinitializing core...";
    ccore.deinit();
    logger(INFO) << "Deinitializing p2p...";
    p2psrv.deinit();

    ccore.set_cryptonote_protocol(NULL);
    cprotocol.set_p2p_endpoint(NULL);
  }
  catch (const std::exception& e)
  {
    logger(ERROR, BRIGHT_RED) << "Exception: " << e.what();
    return 1;
  }

  logger(INFO) << "Node stopped.";
  return 0;
}

bool command_line_preprocessor(const boost::program_options::variables_map& vm, LoggerRef& logger)
{
  bool exit = false;

  if (command_line::get_arg(vm, command_line::arg_version))
  {
    std::cout << cn::CRYPTONOTE_NAME << " v" << PROJECT_VERSION_LONG << ENDL;
    exit = true;
  }

  if (command_line::get_arg(vm, command_line::arg_os_version))
  {
    std::cout << "OS: " << tools::getOSVersion() << ENDL;
    exit = true;
  }

  if (exit)
  {
    return true;
  }

  return false;
}
