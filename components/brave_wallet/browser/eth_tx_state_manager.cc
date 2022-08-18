/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/eth_tx_state_manager.h"

#include <utility>

#include "base/logging.h"
#include "base/strings/strcat.h"
#include "base/values.h"
#include "brave/components/brave_wallet/browser/brave_wallet_constants.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/eip1559_transaction.h"
#include "brave/components/brave_wallet/browser/eip2930_transaction.h"
#include "brave/components/brave_wallet/browser/eth_tx_meta.h"
#include "brave/components/brave_wallet/browser/json_rpc_service.h"
#include "brave/components/brave_wallet/browser/tx_meta.h"
#include "brave/components/brave_wallet/common/eth_address.h"

namespace brave_wallet {

namespace {
constexpr mojom::CoinType kCoinType = mojom::CoinType::ETH;
}

EthTxStateManager::EthTxStateManager(PrefService* prefs,
                                     JsonRpcService* json_rpc_service)
    : TxStateManager(prefs, json_rpc_service) {}

EthTxStateManager::~EthTxStateManager() = default;

std::vector<std::unique_ptr<TxMeta>> EthTxStateManager::GetTransactionsByStatus(
    absl::optional<mojom::TransactionStatus> status,
    absl::optional<EthAddress> from) {
  std::vector<std::unique_ptr<TxMeta>> result;
  absl::optional<std::string> from_string =
      from.has_value() ? absl::optional<std::string>(from->ToChecksumAddress())
                       : absl::nullopt;
  return TxStateManager::GetTransactionsByStatus(status, from_string);
}

std::unique_ptr<EthTxMeta> EthTxStateManager::GetEthTx(const std::string& id) {
  return std::unique_ptr<EthTxMeta>{
      static_cast<EthTxMeta*>(TxStateManager::GetTx(id).release())};
}

std::unique_ptr<EthTxMeta> EthTxStateManager::ValueToEthTxMeta(
    const base::Value::Dict& value) {
  return std::unique_ptr<EthTxMeta>{
      static_cast<EthTxMeta*>(ValueToTxMeta(value).release())};
}

std::unique_ptr<TxMeta> EthTxStateManager::ValueToTxMeta(
    const base::Value::Dict& value) {
  std::unique_ptr<EthTxMeta> meta = std::make_unique<EthTxMeta>();

  if (!TxStateManager::ValueToTxMeta(value, meta.get()))
    return nullptr;

  const base::Value::Dict* tx_receipt = value.FindDict("tx_receipt");
  if (!tx_receipt)
    return nullptr;
  absl::optional<TransactionReceipt> tx_receipt_from_value =
      ValueToTransactionReceipt(*tx_receipt);
  if (!tx_receipt_from_value)
    return nullptr;
  meta->set_tx_receipt(*tx_receipt_from_value);

  const base::Value::Dict* tx = value.FindDict("tx");
  if (!tx)
    return nullptr;
  absl::optional<int> type = tx->FindInt("type");
  if (!type)
    return nullptr;

  switch (static_cast<uint8_t>(*type)) {
    case 0: {
      absl::optional<EthTransaction> tx_from_value =
          EthTransaction::FromValue(*tx);
      if (!tx_from_value)
        return nullptr;
      meta->set_tx(std::make_unique<EthTransaction>(*tx_from_value));
      break;
    }
    case 1: {
      absl::optional<Eip2930Transaction> tx_from_value =
          Eip2930Transaction::FromValue(*tx);
      if (!tx_from_value)
        return nullptr;
      meta->set_tx(std::make_unique<Eip2930Transaction>(*tx_from_value));
      break;
    }
    case 2: {
      absl::optional<Eip1559Transaction> tx_from_value =
          Eip1559Transaction::FromValue(*tx);
      if (!tx_from_value)
        return nullptr;
      meta->set_tx(std::make_unique<Eip1559Transaction>(*tx_from_value));
      break;
    }
    default:
      LOG(ERROR) << "tx type is not supported";
      break;
  }

  return meta;
}

std::string EthTxStateManager::GetTxPrefPathPrefix(
    const absl::optional<std::string>& chain_id) {
  const std::string chain = chain_id ?
      *chain_id : json_rpc_service_->GetChainId(mojom::CoinType::ETH);
  return base::StrCat(
      {kEthereumPrefKey, ".",
       GetNetworkId(prefs_, mojom::CoinType::ETH, chain)});
}

std::vector<std::string> EthTxStateManager::GetTxPrefPathPrefixes(
    const absl::optional<std::string>& chain_id) {
  if (chain_id) {
    return GetTxPrefPathPrefix(*chain_id);
  }

  std::vector<std::string> prefixes;
  for (const auto* network : GetAllChains(prefs_, mojom::CoinType::ETH)) {
    prefixes.push_back(GetTxPrefPathPrefix(network->chain_id));
  }
}

}  // namespace brave_wallet
