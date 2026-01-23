/**************************************************************************
  exif.h  -- A simple ISO C++ library to parse basic EXIF
             information from a JPEG file.

  Based on the description of the EXIF file format at:
  -- http://park2.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html
  -- http://www.media.mit.edu/pia/Research/deepview/exif.html
  -- http://www.exif.org/Exif2-2.PDF

  plenty of changes by co.legoesprit
    Copyright (c) 2010-2016 Mayank Lahiri
  mlahiri@gmail.com
  All rights reserved.

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
#ifndef __EXIF_H
#define __EXIF_H

#include <unordered_map>
#include <QDebug>
#include <string>
#include <sstream>
#include <iomanip>
#include <QString>
#include <QStringList>
#include <QMap>

namespace easyexif {

enum ExifFormat {
    UnsignedByte = 0x01,        // unsigned byte
    AsciiStrings = 0x02,        // ascii strings
    UnsignedShort = 0x03,       // unsigned short
    UnsignedLong = 0x04,        // unsigned long
    UnsignedRational = 0x05,    // unsigned rational
    //SignedByte = 0x06,        // signed byte
    Undefined = 0x07,           // undefined
    //SignedShort = 0x08,       // signed short
    SignedLong = 0x09,          // signed long
    SignedRational = 0x0a,      // signed rational
    //SingleFloat = 0x0b,          // single float
    //SoubleFloat = 0x0c,          // double float
    FF = 0xff                   // unknow entry

};

struct Rational {
    uint32_t numerator, denominator;

    Rational() {
        numerator = 1;
        denominator = 1;
    }

    Rational(uint32_t numerator, uint32_t denominator) {
        this->numerator = numerator;
        this->denominator = denominator;
    }

    explicit Rational(const QString rationalAsString) {
        QStringList asList = rationalAsString.split("/");
        if (asList.count()>1) {
            numerator = asList[0].toLong();
            denominator = asList[1].toLong();
        }
        else {
            numerator = 0;
            denominator = 0;
        }
    }

    operator double() const {
        if (denominator < 1e-20) {
            return 0;
        }
        return static_cast<double>(numerator) / static_cast<double>(denominator);
    };

    Rational operator-() const {
        return Rational(-numerator, denominator);
    }

    double doubleValue() const {
        return *this;
    };

    std::string strValue(int fractionalDigits, std::string suffix = "") const {
        if (denominator < 1e-20) {
            return "0";
        }
        std::ostringstream result;
        result << std::fixed
               << std::setprecision(fractionalDigits)
               << static_cast<double>(numerator) / static_cast<double>(denominator)
               << suffix;
        return result.str();

    };


    std::string ratioF(std::string prefix = "F/") const {
        return prefix+std::to_string(numerator/denominator);
    }

    std::string ratio(std::string suffix = "") const {
        return std::to_string(numerator) + "/" + std::to_string(denominator) +" "+suffix;
    }



};

// IF Entry
class IFEntry {

public:
    using byte_vector = std::vector<uint8_t>;
    using ascii_vector = std::string;
    using short_vector = std::vector<uint16_t>;
    using long_vector = std::vector<uint32_t>;
    using rational_vector = std::vector<Rational>;


    IFEntry(): tag_(0xFF), format_(FF), data_(0), length_(0), val_byte_(nullptr) {}
    //IFEntry(const IFEntry &) = delete;
    IFEntry &operator=(const IFEntry &) = delete;

    ~IFEntry() { delete_union(); }
    unsigned short tag() const { return tag_; }
    void tag(unsigned short tag) { tag_ = tag; }
    ExifFormat format() const { return format_; }
    bool format(unsigned short newFormat) {
        ExifFormat temp = static_cast<ExifFormat>(newFormat);
        switch (temp) {
        case UnsignedByte:            // unsigned byte
        case AsciiStrings:            // ascii strings
        case UnsignedShort:           // unsigned short
        case UnsignedLong:            // unsigned long
        case UnsignedRational:        // unsigned rational
        //case 0x06:                  // signed byte
        case Undefined:               // undefined
        //case 0x08:                  // signed short
        case SignedLong:              // signed long
        case SignedRational:          // signed rational
        //case 0x0b:                  // single float
        //case 0x0c:                  // double float
        case FF:
            break;
        default:
            return false;
        }
        delete_union();
        format_ = temp;
        new_union();
        return true;
    }
    unsigned data() const { return data_; }
    void data(unsigned data) { data_ = data; }
    unsigned length() const { return length_; }
    void length(unsigned length) { length_ = length; }

    // functions to access the data
    //
    // !! it's CALLER responsibility to check that format !!
    // !! is correct before accessing it's field          !!
    //
    // - getters are use here to allow future addition
    //   of checks if format is correct
    byte_vector &val_byte() const { return *val_byte_; }
    ascii_vector &val_string() const { return *val_string_; }
    short_vector &val_short() const { return *val_short_; }
    long_vector &val_long() const { return *val_long_; }
    rational_vector &val_rational() const { return *val_rational_; }


private:
    // Raw fields
    unsigned short tag_;
    ExifFormat format_;
    unsigned data_;
    unsigned length_;

    // Parsed fields
    union {
        byte_vector *val_byte_;
        ascii_vector *val_string_;
        short_vector *val_short_;
        long_vector *val_long_;
        rational_vector *val_rational_;
    };

    void delete_union() {
        switch (format_) {
        case UnsignedByte:
            delete val_byte_;
            val_byte_ = nullptr;
            break;
        case AsciiStrings:
            delete val_string_;
            val_string_ = nullptr;
            break;
        case UnsignedShort:
            delete val_short_;
            val_short_ = nullptr;
            break;
        case UnsignedLong:
            delete val_long_;
            val_long_ = nullptr;
            break;
        case UnsignedRational:
            delete val_rational_;
            val_rational_ = nullptr;
            break;
        case Undefined:
            break;
        case SignedRational:
            delete val_rational_;
            val_rational_ = nullptr;
            break;
        case FF:
            break;
        default:
            assert(false);
            // should not get here
            // should I throw an exception or ...?
            break;
        }
    }
    void new_union() {
        switch (format_) {
        case UnsignedByte:
            val_byte_ = new byte_vector();
            break;
        case AsciiStrings:
            val_string_ = new ascii_vector();
            break;
        case UnsignedShort:
            val_short_ = new short_vector();
            break;
        case UnsignedLong:
            val_long_ = new long_vector();
            break;
        case UnsignedRational:
            val_rational_ = new rational_vector();
            break;
        case Undefined:
            break;
        case SignedRational:
            val_rational_ = new rational_vector();
            break;
        case FF:
            break;
        default:
            assert(false);
            // should not get here
            // should I throw an exception or ...?
            break;
        }
    }
};
//
// Class responsible for storing and parsing EXIF information from a JPEG blob
//
class EXIFInfo {

public:

    struct Geolocation_t {                  // GPS information embedded in file
        double Latitude;                  // Image latitude expressed as decimal
        double Longitude;                 // Image longitude expressed as decimal
        double Altitude;                  // Altitude in meters, relative to sea level
        char AltitudeRef;                 // 0 = above sea level, -1 = below sea level
        double DOP;                       // GPS degree of precision (DOP)
        struct Coord_t {
            double degrees;
            double minutes;
            double seconds;
            char direction;
        } LatComponents, LonComponents;   // Latitude, Longitude expressed in deg/min/sec
    };


    // Parsing function for an entire JPEG image buffer.
    //
    // PARAM 'data': A pointer to a JPEG image.
    // PARAM 'length': The length of the JPEG image.
    // RETURN:  PARSE_EXIF_SUCCESS (0) on succes with 'result' filled out
    //          error code otherwise, as defined by the PARSE_EXIF_ERROR_* macros
    int parseFrom(const unsigned char *data, unsigned length);
    int parseFrom(const std::string &data);

    // Parsing function for an EXIF segment. This is used internally by parseFrom()
    // but can be called for special cases where only the EXIF section is
    // available (i.e., a blob starting with the bytes "Exif\0\0").
    int parseFromEXIFSegment(const unsigned char *buf, unsigned len);

    // Set all data members to default values.
    void clear();


    // Data fields filled out by parseFrom()
    char byteAlign;                   // 0 = Motorola byte alignment, 1 = Intel
    //std::string imageDescription;     // Image description
    //std::string make;                 // Camera manufacturer's name
    //std::string model;                // Camera model
    //unsigned short orientation;       // Image orientation, start of data corresponds to
    // 0: unspecified in EXIF data
    // 1: upper left of image
    // 3: lower right of image
    // 6: upper right of image
    // 8: lower left of image
    // 9: undefined
    //unsigned short bitsPerSample;     // Number of bits per component
    //std::string Software;             // Software used
    //std::string DateTime;             // File change date and time
    //std::string DateTimeOriginal;     // Original file date and time (may not exist)
    //std::string DateTimeDigitized;    // Digitization date and time (may not exist)
    //std::string SubSecTimeOriginal;   // Sub-second time that original picture was taken
    //std::string Copyright;            // File copyright information
    //Rational ExposureTime;            // Exposure time in seconds
    //Rational FNumber;                  // F/stop
    //unsigned short ExposureProgram;   // Exposure program
    // 0: Not defined
    // 1: Manual
    // 2: Normal program
    // 3: Aperture priority
    // 4: Shutter priority
    // 5: Creative program
    // 6: Action program
    // 7: Portrait mode
    // 8: Landscape mode
    //unsigned short ISOSpeedRatings;   // ISO speed
    //Rational ShutterSpeedValue;       // Shutter speed (reciprocal of exposure time)
    //Rational ExposureBiasValue;       // Exposure bias value in EV
    //Rational SubjectDistance;         // Distance to focus point in meters
    //Rational FocalLength;             // Focal length of lens in millimeters
    //unsigned short FocalLengthIn35mm; // Focal length in 35mm film
    //char Flash;                       // 0 = no flash, 1 = flash used
    //unsigned short FlashReturnedLight;// Flash returned light status
    // 0: No strobe return detection function
    // 1: Reserved
    // 2: Strobe return light not detected
    // 3: Strobe return light detected
    //unsigned short FlashMode;         // Flash mode
    // 0: Unknown
    // 1: Compulsory flash firing
    // 2: Compulsory flash suppression
    // 3: Automatic mode
    //unsigned short MeteringMode;      // Metering mode
    // 1: average
    // 2: center weighted average
    // 3: spot
    // 4: multi-spot
    // 5: multi-segment
    //unsigned ImageWidth;              // Image width reported in EXIF data
    //unsigned ImageHeight;             // Image height reported in EXIF data
    //struct Geolocation_t GeoLocation;
    //struct LensInfo_t {               // Lens information
    //    double FStopMin;                // Min aperture (f-stop)
    //    double FStopMax;                // Max aperture (f-stop)
    //    double FocalLengthMin;          // Min focal length (mm)
    //    double FocalLengthMax;          // Max focal length (mm)
    //    Rational FocalPlaneXResolution; // Focal plane X-resolution
    //    Rational FocalPlaneYResolution; // Focal plane Y-resolution
    //    unsigned short FocalPlaneResolutionUnit; // Focal plane resolution unit
    //    // 1: No absolute unit of measurement.
    //    // 2: Inch.
    //    // 3: Centimeter.
    //    // 4: Millimeter.
    //    // 5: Micrometer.
    //    std::string Make;               // Lens manufacturer
    //    std::string Model;              // Lens model
    //} LensInfo;


    EXIFInfo(QByteArray ba);

    EXIFInfo() {
        clear();
    }
private:
    /// valueForKey(key)
    /// like map.at(key), but returns empty std:string in case key does not exist
    std::string valueForKey(const std::string &key) const;

public:
    std::string exposureProgram(short exposure) const;
    std::string focalPlaneResolutionUnit(short unit) const;
    std::string flashReturnedLight(unsigned short FlashReturnedLight) const;
    std::string flashMode(unsigned short FlashMode) const;
    std::string meteringMode(unsigned short meteringMode) const;


    std::string cameraAsString() const;
    std::string titleAsString() const;
    std::string linkLocation() const;
    std::string creatorAsString() const;
    std::string descriptionAsString() const;

private:
    std::unordered_map<std::string, std::string> map;

};

void testCppVariant();

}

// Parse was successful
#define PARSE_EXIF_SUCCESS                    0
// No JPEG markers found in buffer, possibly invalid JPEG file
#define PARSE_EXIF_ERROR_NO_JPEG              1982
// No EXIF header found in JPEG file.
#define PARSE_EXIF_ERROR_NO_EXIF              1983
// Byte alignment specified in EXIF file was unknown (not Motorola or Intel).
#define PARSE_EXIF_ERROR_UNKNOWN_BYTEALIGN    1984
// EXIF header was found, but data was corrupted.
#define PARSE_EXIF_ERROR_CORRUPT              1985

#endif
