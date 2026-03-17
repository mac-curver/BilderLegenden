#include <QDebug>
#include "iptc.h"

IPTC::IPTC(const QByteArray &ba) {
    parseIptcFromUChar(reinterpret_cast<const unsigned char *>(ba.constData()), ba.length());
}



void IPTC::parseIptcFromUChar(const unsigned char *buffer, unsigned int length) {
    if (0 == length)
        return;

    map.clear();
    // Find first marker
    auto findMarker = [&](size_t start) -> long {
        for (size_t i = start; i + 1 < length; ++i) {
            if (buffer[i] == IPTC_ENTRY_MARKER[0] &&
                buffer[i + 1] == IPTC_ENTRY_MARKER[1]) {
                return i;
            }
        }
        return -1;
    };

    long pos = findMarker(0);

    uint8_t lastBlockType = 0x00;
    int count = 0;
    for (; pos != -1 && count < 256; count++) {
        pos = findMarker(pos + IPTC_ENTRY_MARKER.size());
        if (pos == -1) break;

        size_t typePos = pos + IPTC_ENTRY_MARKER.size();
        size_t sizePos = typePos + 1;
        size_t dataPos = sizePos + 2;


        uint8_t blockType = buffer[typePos];

        // Big-endian 16-bit size
        uint16_t blockSize =
            (static_cast<uint16_t>(buffer[sizePos]) << 8) |
            (static_cast<uint16_t>(buffer[sizePos + 1]));

        if (!IPTC_ENTRY_TYPES.count(blockType)) {
            continue;
        }
        if (blockType < lastBlockType) break;
        lastBlockType = blockType;

        if (dataPos + blockSize > MaxIptcSize) {
            break;
            //throw std::runtime_error("Invalid IPTC directory");
        }

        std::string key = IPTC_ENTRY_TYPES.at(blockType);
        std::string value(buffer + dataPos,
                          buffer + dataPos + blockSize);



        map[key] = value;
    }

}

std::string IPTC::valueForKey(const std::string &key) const {
    auto found = map.find(key);
    if (found != map.end()) {
        return found->second;
    }
    else {
        return "";
    }
}

std::string IPTC::cameraAsString() const {
    return "";
}

std::string IPTC::titleAsString() const {
    return valueForKey("title");
}

std::string IPTC::linkLocation() const {
    return valueForKey("rdf:Description@Iptc4xmpCore:Location");
}

std::string IPTC::creatorAsString() const {
    return valueForKey("copyright");
}

std::string IPTC::descriptionAsString() const {
    return valueForKey("caption");
}
