/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <utility>

#include "brave/components/brave_rewards/core/database/database_initialize.h"
#include "brave/components/brave_rewards/core/database/database_util.h"
#include "brave/components/brave_rewards/core/ledger_impl.h"
#include "brave/components/brave_rewards/core/state/state_keys.h"

using std::placeholders::_1;
using std::placeholders::_2;

namespace ledger {
namespace database {

DatabaseInitialize::DatabaseInitialize(LedgerImpl* ledger) : ledger_(ledger) {
  migration_ = std::make_unique<ledger::database::DatabaseMigration>(ledger_);
}

DatabaseInitialize::~DatabaseInitialize() = default;

void DatabaseInitialize::Start(bool execute_create_script,
                               ledger::LegacyResultCallback callback) {
  auto transaction = mojom::DBTransaction::New();
  transaction->version = GetCurrentVersion();
  transaction->compatible_version = GetCompatibleVersion();
  auto command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::INITIALIZE;
  transaction->commands.push_back(std::move(command));

  ledger_->RunDBTransaction(std::move(transaction),
                            std::bind(&DatabaseInitialize::OnInitialize, this,
                                      _1, execute_create_script, callback));
}

void DatabaseInitialize::OnInitialize(mojom::DBCommandResponsePtr response,
                                      bool execute_create_script,
                                      ledger::LegacyResultCallback callback) {
  if (!response ||
      response->status != mojom::DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Response is wrong");
    callback(mojom::Result::DATABASE_INIT_FAILED);
    return;
  }

  if (execute_create_script) {
    GetCreateScript(callback);
    return;
  }

  if (!response->result || !response->result->get_value()->is_int_value()) {
    BLOG(0, "DB init failed");
    callback(mojom::Result::DATABASE_INIT_FAILED);
    return;
  }

  const auto current_table_version =
      response->result->get_value()->get_int_value();
  migration_->Start(current_table_version, callback);
}

void DatabaseInitialize::GetCreateScript(
    ledger::LegacyResultCallback callback) {
  ledger_->client()->GetCreateScript(
      base::BindOnce(&DatabaseInitialize::ExecuteCreateScript,
                     base::Unretained(this), std::move(callback)));
}

void DatabaseInitialize::ExecuteCreateScript(LegacyResultCallback callback,
                                             const std::string& script,
                                             int table_version) {
  if (script.empty()) {
    BLOG(1, "Script is empty");
    callback(mojom::Result::LEDGER_ERROR);
    return;
  }

  ledger_->client()->ClearState(state::kServerPublisherListStamp);

  auto script_callback = std::bind(&DatabaseInitialize::OnExecuteCreateScript,
                                   this, _1, table_version, callback);

  auto transaction = mojom::DBTransaction::New();
  auto command = mojom::DBCommand::New();
  command->type = mojom::DBCommand::Type::EXECUTE;
  command->command = script;
  transaction->commands.push_back(std::move(command));

  ledger_->RunDBTransaction(std::move(transaction), script_callback);
}

void DatabaseInitialize::OnExecuteCreateScript(
    mojom::DBCommandResponsePtr response,
    int table_version,
    ledger::LegacyResultCallback callback) {
  if (!response ||
      response->status != mojom::DBCommandResponse::Status::RESPONSE_OK) {
    BLOG(0, "Response is wrong");
    callback(mojom::Result::DATABASE_INIT_FAILED);
    return;
  }

  migration_->Start(table_version, callback);
}

}  // namespace database
}  // namespace ledger
