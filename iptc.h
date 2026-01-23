#ifndef IPTC_H
#define IPTC_H

#include <cstdint>
#include <vector>
#include <string>
#include <QByteArray>
#include <unordered_map>


class IPTC {
public:
    IPTC(const QByteArray &ba);

private:
    const std::unordered_map<uint8_t, std::string> IPTC_ENTRY_TYPES = {
        {0x05, "title"},
        {0x0F, "category"},
        {0x19, "keywords"},
        {0x37, "dateCreated"},
        {0x3C, "timeCreated"},
        //{0x3E, "dateOther"},
        //{0x3F, "timeOther"},
        {0x50, "byline"},
        {0x55, "bylineTitle"},
        {0x69, "headline"},
        {0x6E, "credit"},
        {0x74, "copyright"},
        {0x78, "caption"},
        {0x7A, "captionWriter"},

    };
    enum IPTC_ENTRIES{
        title = 0x05,
        category = 0x0F,
        keywords = 0x19,
        dateCreated = 0x37,
        timeCreated = 0x3C,
        //dateOther = 0x3E,
        //timeOther = 0x3F,
        byline = 0x50,
        bylineTitle = 0x55,
        headline = 0x69,
        credit = 0x6E,
        copyright = 0x74,
        caption = 0x78,
        captionWriter = 0x7A,
    };

    const std::vector<uint8_t> IPTC_ENTRY_MARKER = {0x1C, 0x02};
    const unsigned int MaxIptcSize = 0x2000;                            // we allow only 0x2000 bytes for the IPTC section

    void parseIptcFromUChar(const unsigned char *buffer, unsigned length);

public:


    std::string cameraAsString() const;
    std::string titleAsString() const;
    std::string linkLocation() const;
    std::string creatorAsString() const;
    std::string descriptionAsString() const;

private:
    /// valueForKey(key)
    /// like map.at(key), but returns empty std:string in case key does not exist
    std::string valueForKey(const std::string &key) const;

    std::unordered_map<std::string, std::string> map;


};

#endif // IPTC_H
