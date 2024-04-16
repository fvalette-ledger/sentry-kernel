// SPDX-FileCopyrightText: 2024 Ledger SAS
// SPDX-License-Identifier: Apache-2.0

/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <string>
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>
#include <argparse/argparse.hpp>

#include <sentry/managers/task.h>
#include <sentry/managers/security.h>
#include <sentry/job.h>

using json = nlohmann::json;

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("genmetadata", "0", argparse::default_arguments::help);

    program.add_argument("-o", "--output").help("generated metadata blob");
    program.add_argument("json_str").help("input json stream");

    try {
        program.parse_args(argc, argv);
        std::istringstream in{program.get<std::string>("json_str")};
        auto data = json::parse(in);

    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    return 0;
}
