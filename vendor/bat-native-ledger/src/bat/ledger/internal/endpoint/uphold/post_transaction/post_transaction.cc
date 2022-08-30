/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ledger/internal/endpoint/uphold/post_transaction/post_transaction.h"

#include <utility>

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/strings/stringprintf.h"
#include "bat/ledger/internal/endpoint/uphold/uphold_utils.h"
#include "bat/ledger/internal/ledger_impl.h"
#include "net/http/http_status_code.h"

namespace ledger::endpoint::uphold {

PostTransaction::PostTransaction(LedgerImpl* ledger)
    : ledger_((DCHECK(ledger), ledger)) {}

PostTransaction::~PostTransaction() = default;

std::string PostTransaction::GetUrl(const std::string& address) {
  const std::string path =
      base::StringPrintf("/v0/me/cards/%s/transactions", address.c_str());

  return GetServerUrl(path);
}

std::string PostTransaction::GeneratePayload(
    const ledger::uphold::Transaction& transaction) {
  base::Value::Dict denomination;
  denomination.Set("amount", base::StringPrintf("%f", transaction.amount));
  denomination.Set("currency", "BAT");

  base::Value::Dict payload;
  payload.Set("destination", transaction.address);
  payload.Set("message", transaction.message);
  payload.Set("denomination", std::move(denomination));

  std::string json;
  base::JSONWriter::Write(payload, &json);
  return json;
}

type::Result PostTransaction::CheckStatusCode(int status_code) {
  if (status_code == net::HTTP_UNAUTHORIZED) {
    BLOG(0, "Unauthorized access");
    return type::Result::EXPIRED_TOKEN;
  }

  if (status_code != net::HTTP_ACCEPTED) {
    BLOG(0, "Unexpected HTTP status: " << status_code);
    return type::Result::LEDGER_ERROR;
  }

  return type::Result::LEDGER_OK;
}

type::Result PostTransaction::ParseBody(const std::string& body,
                                        std::string* transaction_id) {
  DCHECK(transaction_id);

  const auto value = base::JSONReader::Read(body);
  if (!value || !value->is_dict()) {
    BLOG(0, "Invalid body!");
    return type::Result::LEDGER_ERROR;
  }

  const base::Value::Dict& dict = value->GetDict();
  const auto* id = dict.FindString("id");
  if (!id) {
    BLOG(0, "id is missing!");
    return type::Result::LEDGER_ERROR;
  }

  *transaction_id = *id;

  return type::Result::LEDGER_OK;
}

void PostTransaction::Request(const std::string& token,
                              const std::string& address,
                              const ledger::uphold::Transaction& transaction,
                              PostTransactionCallback callback) {
  auto request = type::UrlRequest::New();
  request->url = GetUrl(address);
  request->content = GeneratePayload(transaction);
  request->headers = RequestAuthorization(token);
  request->content_type = "application/json; charset=utf-8";
  request->method = type::UrlMethod::POST;

  ledger_->LoadURL(std::move(request),
                   base::BindOnce(&PostTransaction::OnRequest,
                                  base::Unretained(this), std::move(callback)));
}

void PostTransaction::OnRequest(PostTransactionCallback callback,
                                const type::UrlResponse& response) {
  ledger::LogUrlResponse(__func__, response);

  type::Result result = CheckStatusCode(response.status_code);
  if (result != type::Result::LEDGER_OK) {
    return std::move(callback).Run(result, "");
  }

  std::string transaction_id;
  result = ParseBody(response.body, &transaction_id);
  std::move(callback).Run(result, std::move(transaction_id));
}

}  // namespace ledger::endpoint::uphold
