/*
 * LCD_WebServer
 * Copyright © 2013-2015 Luca Chiodini <luca@chiodini.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Ethernet.h"
#include "LiquidCrystal.h"
#include "SPI.h"
#include "WebServer.h"


// ****************************************************
// ***************** CONFIG SECTION *******************
// ****************************************************

// The number of rows and columns of the LCD.
#define NUM_COLS 20
#define NUM_ROWS 2

// LCD pinout (see <http://arduino.cc/en/Tutorial/LiquidCrystal>).
LiquidCrystal lcd(9, 8, 5, 4, 3, 6);

// MAC address.
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// IP address.
static uint8_t ip[] = { 192, 168, 1, 10 };

// WebServer listening address(es) and port.
WebServer webserver("", 80);

// Default message (i.e. empty LCD)
#define DEFAULT_MSG "No text sent"

// ****************************************************
// ************** END OF CONFIG SECTION ***************
// ****************************************************



// Useful operator to write HTML in the response in C++-like style.
template<class T>
inline Print &operator <<(Print &obj, T arg)
{
    obj.print(arg);
    return obj;
}

String current_shown = "";

void formCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail,
             bool tail_complete)
{
    P(htmlHead) =
        "<html>"
        "<head>"
        "<title>Arduino Web Server</title>"
        "<style type=\"text/css\">"
        "BODY { font-family: sans-serif }"
        "H1 { font-size: 14pt; text-decoration: underline }"
        "P  { font-size: 10pt; }"
        "</style>"
        "</head>"
        "<body>";

    server.httpSuccess();
    server.printP(htmlHead);

    // Post back processing.
    if (type == WebServer::POST)
    {
        // TODO Why are these values hardcoded?
        char name[16], value[40 + 1];
        server.readPOSTparam(name, 16, value, 40 + 1);
        current_shown = value;
        lcd.clear();
        lcd.setCursor(0, 0);
        if (current_shown.length() > 0)
            lcd.print(current_shown);
        else
            lcd.print(DEFAULT_MSG);
        if (current_shown.length() > NUM_COLS)
        {
            lcd.setCursor(0, 1);
            lcd.print(current_shown.substring(NUM_COLS, current_shown.length()));
        }
        server << "Currently showing: \"" << current_shown << "\"\n";
    }
    else // standard page
    {
        server << "Currently showing: ";
        if (current_shown == "")
        {
            server << DEFAULT_MSG << "\n";
            lcd.clear();
            lcd.print(DEFAULT_MSG);
        }
        else
            server << current_shown << "\n";
    }
    server << "<form action='/' method='post'>";
    server << "<input type='text' id='text' name='text'></input><br>";
    server << "<input type='submit' value='Submit'/></form></body></html>";
}

void setup()
{
    // Various initializations.
    lcd.begin(NUM_COLS, NUM_ROWS);
    Ethernet.begin(mac, ip);
    webserver.begin();
    webserver.setDefaultCommand(&formCmd);
    lcd.clear();
    lcd.print(DEFAULT_MSG);
}

void loop()
{
    // Ask the webserver to process incoming requests.
    webserver.processConnection();
}
