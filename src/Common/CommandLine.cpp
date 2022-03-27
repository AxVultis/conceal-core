// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2022 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CommandLine.h"

namespace command_line
{
  const arg_descriptor<bool> arg_help = { "help", "Produce help message" };
  const arg_descriptor<bool> arg_version = { "version", "Output version information" };
  const arg_descriptor<std::string> arg_data_dir = { "data-dir", "Specify data directory" };
  const command_line::arg_descriptor<std::string> arg_config_file = {
    "config-file", "Specify configuration file",
    std::string(cn::BLOCKCHAIN_DIR) + ".conf"
  };
  const command_line::arg_descriptor<bool> arg_os_version = { "os-version", "" };
  const command_line::arg_descriptor<std::string> arg_log_file = { "log-file", "", "" };
  const command_line::arg_descriptor<std::string> arg_set_fee_address = {
    "fee-address", "Set a fee address for remote nodes", ""
  };
  const command_line::arg_descriptor<std::string> arg_set_view_key = {
    "view-key", "Set secret view-key for remote node fee confirmation", ""
  };
  const command_line::arg_descriptor<int> arg_log_level = { "log-level", "", 2 };  // info level
  const command_line::arg_descriptor<bool> arg_console = { "no-console",
                                                           "Disable daemon console commands" };
  const command_line::arg_descriptor<bool> arg_testnet_on = {
    "testnet",
    "Used to deploy test nets. Checkpoints and hardcoded seeds are ignored, "
    "network id is changed. Use it with --data-dir flag. The wallet must be launched with "
    "--testnet flag.",
    false
  };
  const command_line::arg_descriptor<bool> arg_print_genesis_tx = {
    "print-genesis-tx", "Prints genesis' block tx hex to insert it to config and exits"
  };
}  // namespace command_line
