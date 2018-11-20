//
//  ByteBuffer.cc
//  markdownparser
//
//  Created by Zdenek Nemec on 4/22/14.
//  Copyright (c) 2014 Apiary Inc. All rights reserved.
//

#include "ByteBuffer.h"

#include <algorithm>
#include <iterator>
#include <cassert>
#include <iostream>

using namespace mdp;

/* Byte lenght of an UTF8 character (based on first byte) */
#define UTF8_CHAR_LEN(byte) ((0xE5000000 >> ((byte >> 3) & 0x1e)) & 3) + 1

/* Number of UTF8 characters in byte buffer */
static size_t strnlen_utf8(const char* s, size_t len)
{
    if (!s || !len)
        return 0;

    size_t i = 0, j = 0;
    while (i < len && s[i]) {
        i += UTF8_CHAR_LEN(s[i]);
        j++;
    }
    return j;
}

namespace
{
    /* Convert range of bytes to a range of characters */
    CharactersRange BytesRangeToCharactersRange(const BytesRange& bytesRange, const ByteBuffer& byteBuffer)
    {

        std::cerr << "BB " << __LINE__ << std::endl;

        if (byteBuffer.empty()) {
            return CharactersRange{};
        }

        std::cerr << "BB " << __LINE__ << std::endl;

        BytesRange workRange = bytesRange;

        std::cerr << "BB " << __LINE__ << std::endl;

        if (bytesRange.location + bytesRange.length > byteBuffer.length()) {
            // Accomodate maximum possible length
            workRange.length -= bytesRange.location + bytesRange.length - byteBuffer.length();
        }

        std::cerr << "BB " << __LINE__ << std::endl;

        size_t charLocation = 0;
        if (bytesRange.location > 0)
            charLocation = strnlen_utf8(byteBuffer.c_str(), bytesRange.location);

        std::cerr << "BB " << __LINE__ << std::endl;

        size_t charLength = 0;
        if (bytesRange.length > 0)
            charLength = strnlen_utf8(byteBuffer.c_str() + bytesRange.location, bytesRange.length);

        std::cerr << "BB " << __LINE__ << std::endl;

        return CharactersRange(charLocation, charLength);
    }

    CharactersRange BytesRangeToCharactersRange(const BytesRange& bytesRange, const ByteBufferCharacterIndex& index)
    {

        std::cerr << "BB " << __LINE__ << std::endl;

        if (index.empty()) {
            return CharactersRange();
        }

        std::cerr << "BB " << __LINE__ << std::endl;

        BytesRange workRange = bytesRange;

        std::cerr << "BB " << __LINE__ << std::endl;

        if (bytesRange.location + bytesRange.length > index.size()) {
            // Accomodate maximum possible length
            workRange.length -= bytesRange.location + bytesRange.length - index.size();
        }

        std::cerr << "BB " << __LINE__ << std::endl;

        size_t charLocation = 0;
        if (workRange.location > 0)
            charLocation = index[workRange.location];

        std::cerr << "BB " << __LINE__ << std::endl;

        size_t charLength = 0;
        if (workRange.length > 0) {
            size_t pos = workRange.location + workRange.length;
            if (pos >= index.size()) {
                // this code branch is there to be compatioble with strlen_utf8()
                charLength = index[index.size() - 1];
                charLength -= charLocation - 1;
            } else {
                charLength = index[pos];
                charLength -= charLocation;
            }
        }

        std::cerr << "BB " << __LINE__ << std::endl;

        return CharactersRange{ charLocation, charLength };
    }
}

void mdp::BuildCharacterIndex(ByteBufferCharacterIndex& index, const ByteBuffer& byteBuffer)
{

    const char* source = byteBuffer.c_str();
    size_t len = byteBuffer.length();
    size_t pos = 0;
    size_t charPos = 0;

    index.resize(byteBuffer.length());

    while (pos < len && source[pos]) {
        int charLen = UTF8_CHAR_LEN(source[pos]);
        pos += charLen;

        while (charLen) {
            assert(pos - charLen >= 0);
            index[pos - charLen] = charPos;
            charLen--;
        }

        charPos++;
    }
}

CharactersRangeSet mdp::BytesRangeSetToCharactersRangeSet(const BytesRangeSet& rangeSet, const ByteBuffer& byteBuffer)
{
    CharactersRangeSet characterMap{};
    characterMap.reserve(rangeSet.size());

    std::transform(                        //
        rangeSet.begin(),                  //
        rangeSet.end(),                    //
        std::back_inserter(characterMap),  //
        [&byteBuffer](const auto& range) { //
            return BytesRangeToCharactersRange(range, byteBuffer);
        });

    return characterMap;
}

CharactersRangeSet mdp::BytesRangeSetToCharactersRangeSet(
    const BytesRangeSet& rangeSet, const ByteBufferCharacterIndex& index)
{
    std::cerr << "BB " << __LINE__ << std::endl;

    if (rangeSet.empty())
        return {};

    CharactersRangeSet result{};
    result.reserve(rangeSet.size());

    std::cerr << "BB " << __LINE__ << std::endl;
    std::cerr << "BB " << __LINE__ << "; size: " << rangeSet.size() << std::endl;

    std::transform(                   //
        rangeSet.begin(),             //
        rangeSet.end(),               //
        std::back_inserter(result),   //
        [&index](const auto& range) { //
            std::cerr << "BB " << __LINE__ << std::endl;
            return BytesRangeToCharactersRange(range, index);
        });

    std::cerr << "BB " << __LINE__ << std::endl;

    return std::move(result);
}

ByteBuffer mdp::MapBytesRangeSet(const BytesRangeSet& rangeSet, const ByteBuffer& byteBuffer)
{
    if (byteBuffer.empty())
        return ByteBuffer();

    size_t length = byteBuffer.length();
    ByteBufferStream s;
    for (BytesRangeSet::const_iterator it = rangeSet.begin(); it != rangeSet.end(); ++it) {

        if (it->location + it->length > length) {
            // Sundown adds an extra newline on the source input if needed.
            if (it->location + it->length - length == 1) {
                s << byteBuffer.substr(it->location, length - it->location);
                return s.str();
            } else {
                // Wrong map
                return ByteBuffer();
            }
        }

        s << byteBuffer.substr(it->location, it->length);
    }

    return s.str();
}

void mdp::mergeContinuous(RangeSet<Range>& lhs, const RangeSet<Range>& rhs)
{
    if (rhs.empty())
        return;

    if (lhs.empty() || rhs.front().location != lhs.back().location + lhs.back().length) {
        lhs.insert(lhs.end(), rhs.begin(), rhs.end());
    } else {
        // merge
        lhs.back().length += rhs.front().length;

        if (rhs.size() > 1) {
            lhs.insert(lhs.end(), ++rhs.begin(), rhs.end());
        }
    }
}
