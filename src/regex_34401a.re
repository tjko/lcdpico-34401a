/* regex_34401a.re
   Copyright (C) 2026 Timo Kokkonen <tjko@iki.fi>

   SPDX-License-Identifier: GPL-3.0-or-later

   This file is part of LcdPico

   LcdPico is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   LcdPico is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with LcdPico. If not, see <https://www.gnu.org/licenses/>.


   Generate regex.c using: re2c regex_34401a.re -o regex_34401a.c -i --case-ranges
*/

#include <stdio.h>
#include <stdbool.h>
#include "regex_34401a.h"


bool regex_valid_reading(const char *s)
{
	const char *YYCURSOR = s;
	const char *YYMARKER;

/*!local:re2c
	re2c:yyfill:enable = 0;
	re2c:define:YYCTYPE = char;

	prefix = [uUkKmMgG];
	units = ('VDC'|'VAC'|'AAC'|'ADC'|'OHM'|'HZ'|'DB'|'SEC');
	reading = [+-]?[ ]*[0-9]+([.][0-9]+)?[ ]*prefix?units[ ]*;
	misc = [ ]*("O.VLD"[ ]+prefix?units|"OPEN")[ ]*;

	misc    { return true; }
	reading { return true;}
	*	{ return false; }
*/
}


bool regex_in_menu(const char *s)
{
	const char *YYCURSOR = s;
	const char *YYMARKER;

/*!re2c
	re2c:yyfill:enable = 0;
	re2c:define:YYCTYPE = char;

	menutext = [A-Za-Z /-];
	menu = [A-Z][:][ ]+menutext+[ ]*;
	submenu = [0-9][:][ ]+menutext+[ ]*;
	misc = [ ]*('MENUS'|'COMMANDS'|'PARAMETER'|'MENU BOTTOM'|'TOP OF MENU'|'EXITING MENU')[ ]*;

	misc    { return true; }
	menu    { return true; }
	submenu { return true; }
	*	{ return false; }
*/
}


bool regex_text_display(const char *s)
{
	const char *YYCURSOR = s;
//	const char *YYMARKER;

/*!re2c
	re2c:yyfill:enable = 0;
	re2c:define:YYCTYPE = char;

	parameter = [A-Za-Z0-9 .,/+-];
	text = parameter+;

	text    { return true; }
	*	{ return false; }
*/
}

