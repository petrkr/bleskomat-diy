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

#include <string>
#include "config.h"
#include "display.h"
#include "logger.h"
#include "lnurl.h"
#include "modules.h"
#include "modules/dev-console.h"

void setup() {
	Serial.begin(115200);
	logger::enable();
	config::init();
	logger::write("Config OK");
	devConsole::init();
	display::init();
	logger::write("Display OK");
	display::displayBigText(config::defaultDescription);
	coinAcceptor::init();
	coinAcceptor::setFiatCurrency(config::fiatCurrency);
	logger::write("Coin Reader OK");
	logger::write("Setup OK");
}

const unsigned long bootTime = millis();// milliseconds
const unsigned long minWaitAfterBootTime = 3000;// milliseconds
const unsigned long minWaitTimeSinceInsertedFiat = 15000;// milliseconds
const unsigned long maxTimeDisplayQrCode = 180000;// milliseconds

void loop() {
	if (Serial && Serial.available()) {
	    if (devConsole::serialInput()) { devConsole::selectCmd(); }
	};
	if (millis() - bootTime >= minWaitAfterBootTime) {
		// Minimum time has passed since boot.
		// Start performing checks.
		coinAcceptor::loop();
		if (display::getTimeSinceRenderedQRCode() >= maxTimeDisplayQrCode) {
			// Automatically clear the QR code from the screen after some time has passed.
			display::clearLCD();
		} else if (coinAcceptor::coinInserted() && display::hasRenderedQRCode()) {
			// Clear the QR code when new coins are inserted.
			display::clearLCD();
		}
		float accumulatedValue = coinAcceptor::getAccumulatedValue();
		if (
			accumulatedValue > 0 &&
			coinAcceptor::getTimeSinceLastInserted() >= minWaitTimeSinceInsertedFiat
		) {
			printf("accumulatedValue: %f\n", accumulatedValue);
			// The minimum required wait time between coins has passed.
			// Create a withdraw request and render it as a QR code.
			std::string req = lnurl::create_signed_withdraw_request(
				accumulatedValue,
				config::fiatCurrency,
				config::apiKeyId,
				config::apiKeySecret,
				config::callbackUrl,
				config::defaultDescription
			);
			display::clearLCD();
			display::updateAmount(accumulatedValue, config::fiatCurrency);
			display::renderQRCode("lightning:" + req);
			coinAcceptor::reset();
		}
		if ( accumulatedValue > 0 ) {
			if (display::hasShowedIntro()) display::clearLCD();
			display::showAlert(100*coinAcceptor::getTimeSinceLastInserted()/minWaitTimeSinceInsertedFiat);
		} else if (!display::hasShowedIntro() && !display::hasRenderedQRCode()) {
				display::showIntro();
		}
		if ( display::hasRenderedQRCode() ) {
			display::showAlert(100*display::getTimeSinceRenderedQRCode()/maxTimeDisplayQrCode);
		}
		if (!display::hasRenderedQRCode() && display::getRenderedAmount() != accumulatedValue) {
			display::updateBigAmount(accumulatedValue, config::fiatCurrency);
		}
	}
}
