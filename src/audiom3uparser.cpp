//    Copyright (C) 2009 Dirk Vanden Boer <dirk.vdb@gmail.com>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "audio/audiom3uparser.h"

#include "utils/stringoperations.h"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace utils;

namespace audio
{

std::vector<std::string> M3uParser::parseFileContents(const std::string& contents)
{
    std::string copy = contents;
    stringops::dos2unix(copy);
    auto lines = stringops::split(copy, '\n');

    lines.erase(std::remove_if(lines.begin(), lines.end(), [](const std::string& line) {
        return line.find("#") == 0 || stringops::trim(line).empty();
    }),
        lines.end());

    return lines;
}
}
