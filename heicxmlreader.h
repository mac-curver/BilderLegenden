#ifndef HEICXMLREADER_H
#define HEICXMLREADER_H

#include <QMap>
#include <QByteArray>
#include <QDomDocument>

//typedef std::unordered_map<std::string, std::vector<std::string>> AttributesMap;
typedef QMap<QString, QString > AttributesMap;

class HeicXmlReader {
public:

    HeicXmlReader(const QByteArray &ba);

    //std::unordered_map<std::string, std::vector<std::string>> map;
    void extractFromDocument(const QDomDocument &doc);
    void extractElements(const QDomNode &node, AttributesMap &result);

    std::string cameraAsString() const;
    std::string titleAsString() const;
    std::string linkLocation() const;
    std::string creatorAsString() const;
    std::string descriptionAsString() const;

    AttributesMap map;

};

#endif // HEICXMLREADER_H
