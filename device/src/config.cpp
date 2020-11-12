/*
	Copyright (C) 2020 Samotari (Charles Hill, Carlos Garcia Ortiz)
	Contributor: Joe Tinker

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "config.h"

namespace {
        std::string trimQuotes(const std::string &str) {
                return str.substr(1, str.length() - 2);
        }
}

namespace config {
	std::string apiKeyId;
	std::string apiKeySecret;
	std::string callbackUrl;
	std::string fiatCurrency;
	std::string defaultDescription;

	void init() {
		apiKeyId = trimQuotes(STRINGIFY(API_KEY_ID));
		apiKeySecret = trimQuotes(STRINGIFY(API_KEY_SECRET));
		callbackUrl = trimQuotes(STRINGIFY(CALLBACK_URL));
		fiatCurrency = trimQuotes(STRINGIFY(FIAT_CURRENCY));
		defaultDescription = trimQuotes(STRINGIFY(DEFAULT_DESCRIPTION));
	}
}
