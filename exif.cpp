/**************************************************************************
  exif.cpp  -- A simple ISO C++ library to parse basic EXIF
               information from a JPEG file.

  Copyright (c) 2010-2015 Mayank Lahiri
  mlahiri@gmail.com
  All rights reserved (BSD License).

  See exif.h for version history.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  -- Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  -- Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY EXPRESS
  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
  NO EVENT SHALL THE FREEBSD PROJECT OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "exif.h"

#include <QDebug>
#include <QByteArrayMatcher>
#include <algorithm>
#include <cstdint>
#include <stdio.h>

using std::string;

namespace {

using namespace easyexif;






// Helper functions
template <typename T, bool alignIntel>
T parse(const unsigned char *buf);

template <>
uint8_t parse<uint8_t, false>(const unsigned char *buf) {
    return *buf;
}

template <>
uint8_t parse<uint8_t, true>(const unsigned char *buf) {
    return *buf;
}

template <>
uint16_t parse<uint16_t, false>(const unsigned char *buf) {
    return (static_cast<uint16_t>(buf[0]) << 8) |
            static_cast<uint16_t>(buf[1]);
}

template <>
uint16_t parse<uint16_t, true>(const unsigned char *buf) {
    return (static_cast<uint16_t>(buf[1]) << 8) |
            static_cast<uint16_t>(buf[0]);
}

template <>
uint32_t parse<uint32_t, false>(const unsigned char *buf) {
    assert(buf); // proves to analyzer that buf is valid
    return (static_cast<uint32_t>(buf[0]) << 24) |
           (static_cast<uint32_t>(buf[1]) << 16) |
           (static_cast<uint32_t>(buf[2]) << 8)  |
            static_cast<uint32_t>(buf[3]);
}

template <>
uint32_t parse<uint32_t, true>(const unsigned char *buf) {
    assert(buf); // proves to analyzer that buf is valid
    return (static_cast<uint32_t>(buf[3]) << 24) |
           (static_cast<uint32_t>(buf[2]) << 16) |
           (static_cast<uint32_t>(buf[1]) << 8)  |
            static_cast<uint32_t>(buf[0]);
}

template <>
int32_t parse<int32_t, false>(const unsigned char *buf) {
    assert(buf); // proves to analyzer that buf is valid
    return (static_cast<int32_t>(buf[0]) << 24) |
           (static_cast<int32_t>(buf[1]) << 16) |
           (static_cast<int32_t>(buf[2]) << 8)  |
            static_cast<int32_t>(buf[3]);
}

template <>
int32_t parse<int32_t, true>(const unsigned char *buf) {
    assert(buf); // proves to analyzer that buf is valid
    return (static_cast<int32_t>(buf[3]) << 24) |
           (static_cast<int32_t>(buf[2]) << 16) |
           (static_cast<int32_t>(buf[1]) << 8)  |
            static_cast<int32_t>(buf[0]);
}

template <>
Rational parse<Rational, true>(const unsigned char *buf) {
    Rational r;
    r.numerator = parse<uint32_t, true>(buf);
    r.denominator = parse<uint32_t, true>(buf + 4);
    return r;
}

template <>
Rational parse<Rational, false>(const unsigned char *buf) {
    Rational r;
    r.numerator = parse<uint32_t, false>(buf);
    r.denominator = parse<uint32_t, false>(buf + 4);
    return r;
}

/**
 * Try to read entry.length() values for this entry.
 *
 * Returns:
 *  true  - entry.length() values were read
 *  false - something went wrong, vec's content was not touched
 */
template <typename T, bool alignIntel, typename C>
bool extract_values(C &container, const unsigned char *buf, const unsigned base,
                    const unsigned len, const IFEntry &entry) {
    const unsigned char *data;
    uint32_t reversed_data;
    // if data fits into 4 bytes, they are stored directly in
    // the data field in IFEntry
    if (sizeof(T) * entry.length() <= 4) {
        if (alignIntel) {
            reversed_data = entry.data();
        } else {
            reversed_data = entry.data();
            // this reversing works, but is ugly
            unsigned char *rdata = reinterpret_cast<unsigned char *>(&reversed_data);
            unsigned char tmp;
            tmp = rdata[0];
            rdata[0] = rdata[3];
            rdata[3] = tmp;
            tmp = rdata[1];
            rdata[1] = rdata[2];
            rdata[2] = tmp;
        }
        data = reinterpret_cast<const unsigned char *>(&(reversed_data));
    } else {
        data = buf + base + entry.data();
        if (data + sizeof(T) * entry.length() > buf + len) {
            return false;
        }
    }

    container.resize(entry.length());
    for (size_t i = 0; i < entry.length(); ++i) {
        container[i] = parse<T, alignIntel>(data + sizeof(T) * i);
    }

    return true;
}

template <bool alignIntel>
void parseIFEntryHeader(const unsigned char *buf, unsigned short &tag,
                        unsigned short &format, unsigned &length,
                        unsigned &data) {
    // Each directory entry is composed of:
    // 2 bytes: tag number (data field)
    // 2 bytes: data format
    // 4 bytes: number of components
    // 4 bytes: data value or offset to data value
    tag = parse<uint16_t, alignIntel>(buf);
    format = parse<uint16_t, alignIntel>(buf + 2);
    length = parse<uint32_t, alignIntel>(buf + 4);
    data = parse<uint32_t, alignIntel>(buf + 8);
}

template <bool alignIntel>
void parseIFEntryHeader(const unsigned char *buf, IFEntry &result) {
    unsigned short tag;
    unsigned short format;
    unsigned length;
    unsigned data;

    parseIFEntryHeader<alignIntel>(buf, tag, format, length, data);

    result.tag(tag);
    result.format(format);
    result.length(length);
    result.data(data);
}

template <bool alignIntel>
IFEntry parseIFEntry_temp(const unsigned char *buf, const unsigned offs,
                          const unsigned base, const unsigned len) {
    IFEntry result;

    // check if there even is enough data for IFEntry in the buffer
    if (buf + offs + 12 > buf + len) {
        result.tag(0xFF);
        return result;
    }

    parseIFEntryHeader<alignIntel>(buf + offs, result);

    // Parse value in specified format
    switch (result.format()) {
    case UnsignedByte:
        if (!extract_values<uint8_t, alignIntel>(result.val_byte(), buf, base,
                                                 len, result)) {
            result.tag(0xFF);
        }
        break;
    case AsciiStrings:
        // string is basically sequence of uint8_t (well, according to EXIF even
        // uint7_t, but
        // we don't have that), so just read it as bytes
        if (!extract_values<uint8_t, alignIntel>(result.val_string(), buf, base,
                                                 len, result)) {
            result.tag(0xFF);
        }
        // and cut zero byte at the end, since we don't want that in the
        // std::string
        if (result.val_string()[result.val_string().length() - 1] == '\0') {
            result.val_string().resize(result.val_string().length() - 1);
        }
        break;
    case UnsignedShort:
        if (!extract_values<uint16_t, alignIntel>(result.val_short(), buf, base,
                                                  len, result)) {
            result.tag(0xFF);
        }
        break;
    case UnsignedLong:
        if (!extract_values<uint32_t, alignIntel>(result.val_long(), buf, base,
                                                  len, result)) {
            result.tag(0xFF);
        }
        break;
    case UnsignedRational:
        if (!extract_values<Rational, alignIntel>(result.val_rational(), buf,
                                                  base, len, result)) {
            result.tag(0xFF);
        }
        break;
    case Undefined:
        break;
    case SignedLong:
        if (!extract_values<int32_t, alignIntel>(result.val_long(), buf, base,
                                                  len, result)) {
            result.tag(0xFF);
        }
        break;
    case SignedRational:
        if (!extract_values<Rational, alignIntel>(result.val_rational(), buf,
                                                  base, len, result)) {
            result.tag(0xFF);
        }
        break;
    default:
        result.tag(0xFF);
    }
    return result;
}

// helper functions for convinience
template <typename T>
T parse_value(const unsigned char *buf, bool alignIntel) {
    if (alignIntel) {
        return parse<T, true>(buf);
    } else {
        return parse<T, false>(buf);
    }
}

void parseIFEntryHeader(const unsigned char *buf, bool alignIntel,
                        unsigned short &tag, unsigned short &format,
                        unsigned &length, unsigned &data) {
    if (alignIntel) {
        parseIFEntryHeader<true>(buf, tag, format, length, data);
    } else {
        parseIFEntryHeader<false>(buf, tag, format, length, data);
    }
}

IFEntry parseIFEntry(const unsigned char *buf, const unsigned offs,
                     const bool alignIntel, const unsigned base,
                     const unsigned len) {
    if (alignIntel) {
        return parseIFEntry_temp<true>(buf, offs, base, len);
    } else {
        return parseIFEntry_temp<false>(buf, offs, base, len);
    }
}
}

//
// Locates the EXIF segment and parses it using parseFromEXIFSegment
//
int easyexif::EXIFInfo::parseFrom(const unsigned char *buf, unsigned len) {

    // Sanity check: all JPEG files start with 0xFFD8.
    if (!buf || len < 4) return PARSE_EXIF_ERROR_NO_JPEG;
    if (buf[0] != 0xFF || buf[1] != 0xD8) return PARSE_EXIF_ERROR_NO_JPEG;

    // Sanity check: some cameras pad the JPEG image with some bytes at the end.
    // Normally, we should be able to find the JPEG end marker 0xFFD9 at the end
    // of the image buffer, but not always. As long as there are some bytes
    // except 0xD9 at the end of the image buffer, keep decrementing len until
    // an 0xFFD9 is found. If JPEG end marker 0xFFD9 is not found,
    // then we can be reasonably sure that the buffer is not a JPEG.
    while (len > 2) {
        if (buf[len - 1] == 0xD9 && buf[len - 2] == 0xFF) break;
        len--;
    }
    if (len <= 2) {
        return PARSE_EXIF_ERROR_NO_JPEG;
    }

    clear();

    // Scan for EXIF header (bytes 0xFF 0xE1) and do a sanity check by
    // looking for bytes "Exif\0\0". The marker length data is in Motorola
    // byte order, which results in the 'false' parameter to parse16().
    // The marker has to contain at least the TIFF header, otherwise the
    // EXIF data is corrupt. So the minimum length specified here has to be:
    //   2 bytes: section size
    //   6 bytes: "Exif\0\0" string
    //   2 bytes: TIFF header (either "II" or "MM" string)
    //   2 bytes: TIFF magic (short 0x2a00 in Motorola byte order)
    //   4 bytes: Offset to first IFD
    // =========
    //  16 bytes
    unsigned offs = 0;  // current offset into buffer
    for (offs = 0; offs < len - 1; offs++) {
        if (buf[offs] == 0xFF && buf[offs + 1] == 0xE1) break;
    }
    if (offs + 4 > len) return PARSE_EXIF_ERROR_NO_EXIF;
    offs += 2;
    unsigned short section_length = parse_value<uint16_t>(buf + offs, false);
    if (offs + section_length > len || section_length < 16) {
        return PARSE_EXIF_ERROR_CORRUPT;
    }
    offs += 2;
    return parseFromEXIFSegment(buf + offs, len - offs);
}

int easyexif::EXIFInfo::parseFrom(const string &data) {
    return parseFrom(
        reinterpret_cast<const unsigned char *>(data.data()), static_cast<unsigned>(data.length()));
}

//
// Main parsing function for an EXIF segment.
//
// PARAM: 'buf' start of the EXIF TIFF, which must be the bytes "Exif\0\0".
// PARAM: 'len' length of buffer
//
int easyexif::EXIFInfo::parseFromEXIFSegment(
    const unsigned char *buf,
    unsigned len
) {
    bool alignIntel = true;  // byte alignment (defined in EXIF header)
    unsigned offs = 0;       // current offset into buffer
    if (!buf || len < 6) return PARSE_EXIF_ERROR_NO_EXIF;

    if (!std::equal(buf, buf + 6, "Exif\0\0")) return PARSE_EXIF_ERROR_NO_EXIF;
    offs += 6;

    // Now parsing the TIFF header. The first two bytes are either "II" or
    // "MM" for Intel or Motorola byte alignment. Sanity check by parsing
    // the unsigned short that follows, making sure it equals 0x2a. The
    // last 4 bytes are an offset into the first IFD, which are added to
    // the global offset counter. For this block, we expect the following
    // minimum size:
    //  2 bytes: 'II' or 'MM'
    //  2 bytes: 0x002a
    //  4 bytes: offset to first IDF
    // -----------------------------
    //  8 bytes
    if (offs + 8 > len) return PARSE_EXIF_ERROR_CORRUPT;
    unsigned tiff_header_start = offs;
    if (buf[offs] == 'I' && buf[offs + 1] == 'I') {
        alignIntel = true;
    }
    else {
        if (buf[offs] == 'M' && buf[offs + 1] == 'M') {
            alignIntel = false;
        }
        else {
            return PARSE_EXIF_ERROR_UNKNOWN_BYTEALIGN;
        }
    }
    this->byteAlign = alignIntel;
    offs += 2;
    if (0x2a != parse_value<uint16_t>(buf + offs, alignIntel)) {
        return PARSE_EXIF_ERROR_CORRUPT;
    }
    offs += 2;
    unsigned first_ifd_offset = parse_value<uint32_t>(buf + offs, alignIntel);
    offs += first_ifd_offset - 4;
    if (offs >= len) return PARSE_EXIF_ERROR_CORRUPT;

    // Now parsing the first Image File Directory (IFD0, for the main image).
    // An IFD consists of a variable number of 12-byte directory entries. The
    // first two bytes of the IFD section contain the number of directory
    // entries in the section. The last 4 bytes of the IFD contain an offset
    // to the next IFD, which means this IFD must contain exactly 6 + 12 * num
    // bytes of data.
    if (offs + 2 > len) return PARSE_EXIF_ERROR_CORRUPT;
    int num_entries = parse_value<uint16_t>(buf + offs, alignIntel);
    if (offs + 6 + 12 * num_entries > len) return PARSE_EXIF_ERROR_CORRUPT;
    offs += 2;
    unsigned exif_sub_ifd_offset = len;
    unsigned gps_sub_ifd_offset = len;
    while (--num_entries >= 0) {
        IFEntry result = parseIFEntry(buf, offs, alignIntel, tiff_header_start, len);
        offs += 12;
        switch (result.tag()) {
        case 0x102:
            // Bits per sample
            if (result.format() == UnsignedShort && result.val_short().size()) {
                //this->bitsPerSample = result.val_short().front();
                map["Bits/sample"] = std::to_string(result.val_short().front());
            }
            break;

        case 0x10E:
            // Image description
            if (result.format() == AsciiStrings) {
                //this->imageDescription = result.val_string();
                map["Image Description"] = result.val_string();
            }
            break;

        case 0x10F:
            // Digicam make
            if (result.format() == AsciiStrings) {
                //this->make = result.val_string();
                map["Camera Make"] = result.val_string();
            }
            break;

        case 0x110:
            // Digicam model
            if (result.format() == AsciiStrings) {
                //this->model = result.val_string();
                map["Camera Model"] = result.val_string();
            }
            break;

        case 0x112:
            // Orientation of image
            if (result.format() == UnsignedShort && result.val_short().size()) {
                //this->orientation = result.val_short().front();
                map["Orientation"] = std::to_string(result.val_short().front());
            }
            break;

        case 0x131:
            // Software used for image
            if (result.format() == AsciiStrings) {
                //this->Software = result.val_string();
                map["Software"] = result.val_string();
            }
            break;

        case 0x132:
            // EXIF/TIFF date/time of image modification
            if (result.format() == AsciiStrings) {
                //this->DateTime = result.val_string();
                map["Date/time"] = result.val_string();
            }
            break;

        case 0x8298:
            // Copyright information
            if (result.format() == AsciiStrings) {
                //this->Copyright = result.val_string();
                map["Copyright"] = result.val_string();
            }
            break;

        case 0x8825:
            // GPS IFS offset
            gps_sub_ifd_offset = tiff_header_start + result.data();
            break;

        case 0x8769:
            // EXIF SubIFD offset
            exif_sub_ifd_offset = tiff_header_start + result.data();
            break;
        }
    }

    // Jump to the EXIF SubIFD if it exists and parse all the information
    // there. Note that it's possible that the EXIF SubIFD doesn't exist.
    // The EXIF SubIFD contains most of the interesting information that a
    // typical user might want.
    if (exif_sub_ifd_offset + 4 <= len) {
        offs = exif_sub_ifd_offset;
        int num_sub_entries = parse_value<uint16_t>(buf + offs, alignIntel);
        if (offs + 6 + 12 * num_sub_entries > len) return PARSE_EXIF_ERROR_CORRUPT;
        offs += 2;
        while (--num_sub_entries >= 0) {
            IFEntry result = parseIFEntry(buf, offs, alignIntel, tiff_header_start, len);
            switch (result.tag()) {
            case 0x829a:
                // Exposure time in seconds
                if ((result.format() == UnsignedRational || result.format() == SignedRational) && result.val_rational().size()) {
                    //this->ExposureTime = result.val_rational().front();
                    map["ExposureTime"] = result.val_rational().front().ratio(" s");
                }
                break;

            case 0x829d:
                // FNumber
                if ((result.format() == UnsignedRational || result.format() == SignedRational) && result.val_rational().size()) {
                    //this->FNumber = result.val_rational().front();
                    map["FNumber"] = result.val_rational().front().ratioF();
                }
                break;

            case 0x8822:
                // Exposure Program
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->ExposureProgram = result.val_short().front();
                    map["Exposure Program"] = exposureProgram(result.val_short().front());
                }
                break;

            case 0x8827:
                // ISO Speed Rating
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->ISOSpeedRatings = result.val_short().front();
                    map["ISO Rating"] = std::to_string(result.val_short().front());
                }
                break;

            case 0x9003:
                // Original date and time
                if (result.format() == AsciiStrings) {
                    //this->DateTimeOriginal = result.val_string();
                    map["Date/Time org."] = result.val_string();
                }
                break;

            case 0x9004:
                // Digitization date and time
                if (result.format() == AsciiStrings) {
                    //this->DateTimeDigitized = result.val_string();
                    map["Date/Time digitized"] = result.val_string();

                }
                break;

            case 0x9201:
                // Shutter speed value
                if ((result.format() == UnsignedRational || result.format() == SignedRational) && result.val_rational().size()) {
                    //this->ShutterSpeedValue = result.val_rational().front();
                    map["Shutter Speed Value"] = result.val_rational().front().ratio(" s");
                }
                break;

            case 0x9204:
                // Exposure bias value
                if ((result.format() == UnsignedRational|| result.format() == SignedRational) && result.val_rational().size()) {
                    //this->ExposureBiasValue = result.val_rational().front();
                    map["Exposure Bias Value"] = result.val_rational().front().ratio();
                }
                break;

            case 0x9206:
                // Subject distance
                if ((result.format() == UnsignedRational || result.format() == SignedRational) && result.val_rational().size()) {
                    //this->SubjectDistance = result.val_rational().front();
                    map["Subject Distance"] = result.val_rational().front().ratio();
                }
                break;

            case 0x9209:
                // Flash used
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    uint16_t data = result.val_short().front();

                    //this->Flash = data & 1;
                    //this->FlashReturnedLight = (data & 6) >> 1;
                    //this->FlashMode = (data & 24) >> 3;

                    map["Flash"] = std::to_string(data & 1);
                    map["Flash Returned Light"] = flashReturnedLight((data & 6) >> 1);
                    map["FlashMode"] = flashMode((data & 24) >> 3);

                }
                break;

            case 0x920a:
                // Focal length
                if ((result.format() == UnsignedRational|| result.format() == SignedRational) && result.val_rational().size()) {
                    //this->FocalLength = result.val_rational().front();
                    map["Focal Length"] = result.val_rational().front().strValue(0, "mm");
                }
                break;

            case 0x9207:
                // Metering mode
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->MeteringMode = result.val_short().front();
                    map["Metering Mode"] = meteringMode(result.val_short().front());
                }
                break;

            case 0x9291:
                // Subsecond original time
                if (result.format() == AsciiStrings) {
                    //this->SubSecTimeOriginal = result.val_string();
                    map["Subsecond org."] = result.val_string();
                }
                break;

            case 0xa002:
                // EXIF Image width
                if (result.format() == UnsignedLong && result.val_long().size()) {
                    //this->ImageWidth = result.val_long().front();
                    map["Image Width"] = std::to_string(result.val_long().front());
                }
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->ImageWidth = result.val_short().front();
                    map["Image Width"] = std::to_string(result.val_short().front());
                }
                break;

            case 0xa003:
                // EXIF Image height
                if (result.format() == UnsignedLong && result.val_long().size()) {
                    //this->ImageHeight = result.val_long().front();
                    map["Image Height"] = std::to_string(result.val_long().front());
                }
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->ImageHeight = result.val_short().front();
                    map["Image Height"] = std::to_string(result.val_short().front());
                }
                break;

            case 0xa20e:
                // EXIF Focal plane X-resolution
                if ((result.format() == UnsignedRational || result.format() == SignedRational)) {
                    //this->LensInfo.FocalPlaneXResolution = result.val_rational()[0];
                    map["Lens.Focal Plane X Resolution"] = result.val_rational().front().strValue(0, "cm");
                }
                break;

            case 0xa20f:
                // EXIF Focal plane Y-resolution
                if ((result.format() == UnsignedRational || result.format() == SignedRational)) {
                    //this->LensInfo.FocalPlaneYResolution = result.val_rational()[0];
                    map["Lens.Focal Plane Y Resolution"] = result.val_rational().front().strValue(0, "cm");
                }
                break;

            case 0xa210:
                // EXIF Focal plane resolution unit
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->LensInfo.FocalPlaneResolutionUnit = result.val_short().front();
                    map["Lens.Focal plane resolution unit"] = focalPlaneResolutionUnit(result.val_short().front());
                }
                break;

            case 0xa405:
                // Focal length in 35mm film
                if (result.format() == UnsignedShort && result.val_short().size()) {
                    //this->FocalLengthIn35mm = result.val_short().front();
                    map["Lens.35mm Equivalent"] = std::to_string(result.val_short().front()) + " mm";
                }
                break;

            case 0xa432:
                // Focal length and FStop.
                if (result.format() == UnsignedRational) {
                    int sz = static_cast<unsigned>(result.val_rational().size());
                    if (sz) {
                        //this->LensInfo.FocalLengthMin = result.val_rational()[0];
                        map["Lens.Focal Length Min"] = result.val_rational()[0].strValue(0, "mm");
                    }
                    if (sz > 1) {
                        //this->LensInfo.FocalLengthMax = result.val_rational()[1];
                        map["Lens.Focal Length Max"] = result.val_rational()[1].strValue(0, "mm");
                    }
                    if (sz > 2) {
                        //this->LensInfo.FStopMin = result.val_rational()[2];
                        map["Lens.FStop Min"] = result.val_rational()[2].strValue(1, "");
                    }
                    if (sz > 3) {
                        //this->LensInfo.FStopMax = result.val_rational()[3];
                        map["Lens.FStop Max"] = result.val_rational()[3].strValue(1, "");
                    }
                }
                break;

            case 0xa433:
                // Lens make.
                if (result.format() == AsciiStrings) {
                    //this->LensInfo.Make = result.val_string();
                    map["Lens.Make"] = result.val_string();
                }
                break;

            case 0xa434:
                // Lens model.
                if (result.format() == AsciiStrings) {
                    //this->LensInfo.Model = result.val_string();
                    map["Lens.Model"] = result.val_string();
                }
                break;
            }
            offs += 12;
        }
    }

    // Jump to the GPS SubIFD if it exists and parse all the information
    // there. Note that it's possible that the GPS SubIFD doesn't exist.
    if (gps_sub_ifd_offset + 4 <= len) {
        offs = gps_sub_ifd_offset;
        int num_sub_entries = parse_value<uint16_t>(buf + offs, alignIntel);
        if (offs + 6 + 12 * num_sub_entries > len) return PARSE_EXIF_ERROR_CORRUPT;
        offs += 2;
        Geolocation_t geo;
        geo.LatComponents.degrees = 0;
        geo.LatComponents.minutes = 0;
        geo.LatComponents.seconds = 0;
        geo.LatComponents.direction = '?';
        geo.LonComponents.degrees = 0;
        geo.LonComponents.minutes = 0;
        geo.LonComponents.seconds = 0;
        geo.LonComponents.direction = '?';
        char altitudeRef = '0';
        while (--num_sub_entries >= 0) {
            unsigned short tag, format;
            unsigned length, data;
            parseIFEntryHeader(buf + offs, alignIntel, tag, format, length, data);
            switch (tag) {
            case 1:
                // GPS north or south
                //this->GeoLocation.LatComponents.direction = *(buf + offs + 8);
                //if (this->GeoLocation.LatComponents.direction == 0) {
                //    this->GeoLocation.LatComponents.direction = '?';
                //}
                //if ('S' == this->GeoLocation.LatComponents.direction) {
                //    this->GeoLocation.Latitude = -this->GeoLocation.Latitude;
                //}
                geo.LatComponents.direction = *(buf + offs + 8);

                break;

            case 2:
                // GPS latitude
                if ((format == UnsignedRational || format == SignedRational) && length == 3) {
                    //this->GeoLocation.LatComponents.degrees = parse_value<Rational>(
                    //    buf + data + tiff_header_start, alignIntel);
                    //this->GeoLocation.LatComponents.minutes = parse_value<Rational>(
                    //    buf + data + tiff_header_start + 8, alignIntel);
                    //this->GeoLocation.LatComponents.seconds = parse_value<Rational>(
                    //    buf + data + tiff_header_start + 16, alignIntel);
                    //this->GeoLocation.Latitude =
                    //    this->GeoLocation.LatComponents.degrees +
                    //    this->GeoLocation.LatComponents.minutes / 60 +
                    //    this->GeoLocation.LatComponents.seconds / 3600;
                    //if ('S' == this->GeoLocation.LatComponents.direction) {
                    //    this->GeoLocation.Latitude = -this->GeoLocation.Latitude;
                    //}
                    geo.LatComponents.degrees = parse_value<Rational>(
                        buf + data + tiff_header_start, alignIntel);
                    geo.LatComponents.minutes = parse_value<Rational>(
                        buf + data + tiff_header_start + 8, alignIntel);
                    geo.LatComponents.seconds = parse_value<Rational>(
                        buf + data + tiff_header_start + 16, alignIntel);
                    geo.Latitude = geo.LatComponents.degrees
                                   + geo.LatComponents.minutes /60
                                   + geo.LatComponents.seconds /3600;
                    if ('S' == geo.LatComponents.direction) {
                        geo.Latitude = -geo.Latitude;
                    }
                }
                break;

            case 3:
                // GPS east or west
                //this->GeoLocation.LonComponents.direction = *(buf + offs + 8);
                //if (this->GeoLocation.LonComponents.direction == 0) {
                //    this->GeoLocation.LonComponents.direction = '?';
                //}
                //if ('W' == this->GeoLocation.LonComponents.direction) {
                //    this->GeoLocation.Longitude = -this->GeoLocation.Longitude;
                //}
                geo.LonComponents.direction = *(buf + offs + 8);

                break;

            case 4:
                // GPS longitude
                if ((format == UnsignedRational || format == SignedRational) && length == 3) {
                    //this->GeoLocation.LonComponents.degrees = parse_value<Rational>(
                    //    buf + data + tiff_header_start, alignIntel);
                    //this->GeoLocation.LonComponents.minutes = parse_value<Rational>(
                    //    buf + data + tiff_header_start + 8, alignIntel);
                    //this->GeoLocation.LonComponents.seconds = parse_value<Rational>(
                    //    buf + data + tiff_header_start + 16, alignIntel);
                    //this->GeoLocation.Longitude =
                    //    this->GeoLocation.LonComponents.degrees +
                    //    this->GeoLocation.LonComponents.minutes / 60 +
                    //    this->GeoLocation.LonComponents.seconds / 3600;
                    //if ('W' == this->GeoLocation.LonComponents.direction)
                    //    this->GeoLocation.Longitude = -this->GeoLocation.Longitude;
                    geo.LonComponents.degrees = parse_value<Rational>(
                        buf + data + tiff_header_start, alignIntel);
                    geo.LonComponents.minutes = parse_value<Rational>(
                        buf + data + tiff_header_start + 8, alignIntel);
                    geo.LonComponents.seconds = parse_value<Rational>(
                        buf + data + tiff_header_start + 16, alignIntel);
                    geo.Longitude = geo.LonComponents.degrees
                                   + geo.LonComponents.minutes /60
                                   + geo.LonComponents.seconds /3600;
                    if ('W' == geo.LonComponents.direction) {
                        geo.Longitude = -geo.Longitude;
                    }
                }

                break;

            case 5:
                // GPS altitude reference (below or above sea level)
                //this->GeoLocation.AltitudeRef = *(buf + offs + 8);
                //if (1 == this->GeoLocation.AltitudeRef) {
                //    this->GeoLocation.Altitude = -this->GeoLocation.Altitude;
                //}
                altitudeRef = *(buf + offs + 8);
                break;

            case 6:
                // GPS altitude
                if ((format == UnsignedRational || format == SignedRational)) {
                    //this->GeoLocation.Altitude = parse_value<Rational>(
                    //    buf + data + tiff_header_start, alignIntel);
                    //if (1 == this->GeoLocation.AltitudeRef) {
                    //    this->GeoLocation.Altitude = -this->GeoLocation.Altitude;
                    //}
                    Rational height = parse_value<Rational>(
                        buf + data + tiff_header_start, alignIntel);
                    if (1 == altitudeRef) {
                        height = -height;
                    }
                    map["Geo.Altitude"] = height.strValue(1, " m");
                }
                break;

            case 11:
                // GPS degree of precision (DOP)
                if ((format == UnsignedRational || format == SignedRational)) {
                    //this->GeoLocation.DOP = parse_value<Rational>(
                    //    buf + data + tiff_header_start, alignIntel);

                    Rational dop = parse_value<Rational>(
                        buf + data + tiff_header_start, alignIntel);
                    map["Geo.Dop"] = dop.ratio();
                }
                break;
            }
            offs += 12;
        }
        std::ostringstream latitudeStream;
        latitudeStream
            << geo.LatComponents.direction << " "
            << geo.LatComponents.degrees << "°"
            << geo.LatComponents.minutes << "'"
            << std::setprecision(5)
            << geo.LatComponents.seconds;
        std::ostringstream longitudeStream;
        longitudeStream
            << geo.LonComponents.direction << " "
            << geo.LonComponents.degrees << "°"
            << geo.LonComponents.minutes << "'"
            << std::setprecision(5)
            << geo.LonComponents.seconds;

        map["Latitude"] = latitudeStream.str();
        map["Longitude"] = longitudeStream.str();
    }

    return PARSE_EXIF_SUCCESS;
}

void easyexif::EXIFInfo::clear() {
    // Strings
    //imageDescription = "";
    //make = "";
    //model = "";
    //Software = "";
    //DateTime = "";
    //DateTimeOriginal = "";
    //DateTimeDigitized = "";
    //SubSecTimeOriginal = "";
    //Copyright = "";

    // Shorts / unsigned / double
    byteAlign = 0;
    //orientation = 0;

    //bitsPerSample = 0;
    //ExposureTime = {0, 0};
    //FNumber = {0, 0};
    //ExposureProgram = 0;
    //ISOSpeedRatings = 0;
    //ShutterSpeedValue = {0, 0};
    //ExposureBiasValue = {0, 0};
    //SubjectDistance = {0, 0};
    //FocalLength = {0, 0};
    //FocalLengthIn35mm = 0;
    //Flash = 0;
    //FlashReturnedLight = 0;
    //FlashMode = 0;
    //MeteringMode = 0;
    //ImageWidth = 0;
    //ImageHeight = 0;

    // Geolocation
    //GeoLocation.Latitude = 0;
    //GeoLocation.Longitude = 0;
    //GeoLocation.Altitude = 0;
    //GeoLocation.AltitudeRef = 0;
    //GeoLocation.DOP = 0;
    //GeoLocation.LatComponents.degrees = 0;
    //GeoLocation.LatComponents.minutes = 0;
    //GeoLocation.LatComponents.seconds = 0;
    //GeoLocation.LatComponents.direction = '?';
    //GeoLocation.LonComponents.degrees = 0;
    //GeoLocation.LonComponents.minutes = 0;
    //GeoLocation.LonComponents.seconds = 0;
    //GeoLocation.LonComponents.direction = '?';

    // LensInfo
    //LensInfo.FocalLengthMax = 0;
    //LensInfo.FocalLengthMin = 0;
    //LensInfo.FStopMax = 0;
    //LensInfo.FStopMin = 0;
    //LensInfo.FocalPlaneYResolution = {0, 0};
    //LensInfo.FocalPlaneXResolution = {0, 0};
    //LensInfo.FocalPlaneResolutionUnit = 0;
    //LensInfo.Make = "";
    //LensInfo.Model = "";
}

std::string EXIFInfo::exposureProgram(short exposure) const {
    switch (exposure) {
    case 1: return "Manual";
    case 2: return "Normal program";
    case 3: return "Aperture priority";
    case 4: return "Shutter priority";
    case 5: return "Creative program";
    case 6: return "Action program";
    case 7: return "Portrait mode";
    case 8: return "Landscape mode";
    default: return "Not defined";
    }
}



string EXIFInfo::flashReturnedLight(unsigned short flashReturnedLight) const {
    //unsigned short FlashReturnedLight;// Flash returned light status
    switch (flashReturnedLight) {
    case 0: return "No strobe detection function";
    case 1: return "Reserved";
    case 2: return "Strobe return light not detected";
    case 3: return "Strobe return light detected";
    default: return "unknown";
    }

}

string EXIFInfo::flashMode(unsigned short flashMode) const {
    //unsigned short FlashMode;         // Flash mode
    switch (flashMode) {
    case 1: return "Compulsory flash firing";
    case 2: return "Compulsory flash suppression";
    case 3: return "Automatic mode";
    case 0:
    default: return "Unknown";
    }

}

EXIFInfo::EXIFInfo(QByteArray ba) {

    int code = parseFrom(reinterpret_cast<unsigned char*>(ba.data()), ba.length());
    if (code) {
        QByteArrayMatcher matchStart("Exif\0\0");
        qsizetype start = matchStart.indexIn(ba);
        start = matchStart.indexIn(ba, start+1); // 2nd Exif occurence
        if (0 < start) {
            qsizetype len = 0x1000;
            QByteArray exifBa = ba.sliced(start, len);
            code = parseFromEXIFSegment(reinterpret_cast<unsigned char*>(exifBa.data()), qMin(len, ba.length()));
        }
        else {
            code = -1;
        }
        if (code) {
            //QMessageBox::critical(this, "Parsing Exif not successful", "Can't find Exif information.");
        }
    }
}

string EXIFInfo::valueForKey(const std::string &key) const {
    auto found = map.find(key);
    if (found != map.end()) {
        return found->second;
    }
    else {
        return "";
    }
}

string EXIFInfo::meteringMode(unsigned short meteringMode) const {
    switch (meteringMode) {
    case 1: return "average";
        case 2: return "center weighted average";
        case 3: return "spot";
        case 4: return "multi-spot";
        case 5: return "multi-segment";
            default: return "Undefined";
    }

}

string EXIFInfo::focalPlaneResolutionUnit(short unit) const {
    //    unsigned short FocalPlaneResolutionUnit; // Focal plane resolution unit
    switch (unit) {
    case 2: return "\"";
    case 3: return "cm";
    case 4: return "mm";
    case 5: return "um";
    case 1:
    default:
        return "No absolute unit of measurement";

    }
}

void testCppVariant() {
    std::variant<int, double, std::string, easyexif::Rational> a, b, c, d;
    a = 23;
    b = 67.8;
    c = "Dies ist ein Test";
    d = easyexif::Rational(5, 3);

    //qDebug() << "testCppVariant" << std::get<int>(a) << std::get<double>(b) << std::get<std::string>(c);
    //qDebug() << "testCppVariant" << std::get<easyexif::Rational>(d).numerator << std::get<easyexif::Rational>(d).denominator;

}


string EXIFInfo::cameraAsString() const {
    std::ostringstream result;

    result << valueForKey("Camera Make").c_str();
    result << " ";
    result << valueForKey("Camera Model").c_str();
    result << " ";
    result << valueForKey("Lens.Model").c_str();
    result << " ";
    result << valueForKey("Lens.35mm Equivalent");
    result << " ";
    result << valueForKey("FNumber").c_str();
    result << " ";
    result << valueForKey("ExposureTime").c_str();
    return result.str();
}

string EXIFInfo::titleAsString() const {
    return "";
}

std::string easyexif::EXIFInfo::linkLocation() const {
    return "";
}


string EXIFInfo::creatorAsString() const {
    return valueForKey("Copyright").c_str();
}

string EXIFInfo::descriptionAsString() const {
    return valueForKey("Image Description").c_str();
}
