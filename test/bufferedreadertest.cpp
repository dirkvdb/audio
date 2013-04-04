//    Copyright (C) 2012 Dirk Vanden Boer <dirk.vdb@gmail.com>
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

#include "gtest/gtest.h"

#include <array>
#include <iostream>
#include <algorithm>


#include "audio/audiobufferedreader.h"
#include "audio/audiofilereader.h"
#include "utils/fileoperations.h"

using namespace utils;
using namespace testing;

namespace audio
{
namespace test
{

static const std::string g_testFile = "bufferedreadertestfile.bin";

class BufferedReaderTest : public Test
{
public:
    BufferedReaderTest()
    : reader(fileReader, 10)
    {
    }

protected:
    void SetUp()
    {
        try { fileops::deleteFile(g_testFile); } catch (std::exception&) {}
        std::ofstream file(g_testFile, std::ios::binary);
        EXPECT_TRUE(file.is_open());
        
        data.resize(100);
        for (int i = 0; i < 100; ++i)
        {
            data[i] = i;
        }
        
        file.write(reinterpret_cast<char*>(data.data()), data.size());
        file.close();
        
        EXPECT_NO_THROW(reader.open(g_testFile));
    }
    
    void TearDown()
    {
        EXPECT_NO_THROW(fileops::deleteFile(g_testFile));
    }
    
    FileReader              fileReader;
    BufferedReader          reader;
    std::vector<uint8_t>    data;
};

TEST_F(BufferedReaderTest, contentLength)
{
    EXPECT_EQ(100, reader.getContentLength());
}

TEST_F(BufferedReaderTest, readFullBuffer)
{
    std::vector<uint8_t> readData(data.size(), '\0');
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    
    EXPECT_EQ(0, memcmp(data.data(), readData.data(), data.size()));
    EXPECT_TRUE(reader.eof());
}

TEST_F(BufferedReaderTest, readSmallData)
{
    std::array<uint8_t, 2> readData;
    
    reader.seekAbsolute(3);
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(3, readData[0]);
    EXPECT_EQ(4, readData[1]);
    
    reader.seekRelative(2);
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(7, readData[0]);
    EXPECT_EQ(8, readData[1]);
    
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(9, readData[0]);
    EXPECT_EQ(10, readData[1]);
    
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(11, readData[0]);
    EXPECT_EQ(12, readData[1]);
    
    reader.seekAbsolute(55);
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(55, readData[0]);
    EXPECT_EQ(56, readData[1]);
    
    EXPECT_FALSE(reader.eof());
}

TEST_F(BufferedReaderTest, readOutsideExistingBuffer)
{
    std::array<uint8_t, 6> readData;
    
    reader.seekRelative(5);
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(5,  readData[0]);
    EXPECT_EQ(6,  readData[1]);
    EXPECT_EQ(7,  readData[2]);
    EXPECT_EQ(8,  readData[3]);
    EXPECT_EQ(9,  readData[4]);
    EXPECT_EQ(10, readData[5]);
    
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(11, readData[0]);
    EXPECT_EQ(12, readData[1]);
    EXPECT_EQ(13, readData[2]);
    EXPECT_EQ(14, readData[3]);
    EXPECT_EQ(15, readData[4]);
    EXPECT_EQ(16, readData[5]);
    
    EXPECT_FALSE(reader.eof());
}

TEST_F(BufferedReaderTest, bigReadAfterSmallRead)
{
    std::array<uint8_t, 2> readData;
    
    reader.seekAbsolute(50);
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(50,  readData[0]);
    EXPECT_EQ(51,  readData[1]);
    
    std::array<uint8_t, 12> bigReadData;
    EXPECT_EQ(bigReadData.size(), reader.read(bigReadData.data(), bigReadData.size()));
    EXPECT_EQ(0, memcmp(data.data() + 52, bigReadData.data(), bigReadData.size()));
    
    EXPECT_EQ(readData.size(), reader.read(readData.data(), readData.size()));
    EXPECT_EQ(64,  readData[0]);
    EXPECT_EQ(65,  readData[1]);
    
    EXPECT_FALSE(reader.eof());
}

TEST_F(BufferedReaderTest, endOfFile)
{
    std::array<uint8_t, 2> readData;
    
    reader.seekAbsolute(99);
    EXPECT_EQ(1, reader.read(readData.data(), readData.size()));
    EXPECT_EQ(99,  readData[0]);
    
    EXPECT_TRUE(reader.eof());
}

}
}
