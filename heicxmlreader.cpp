#include <QDebug>
#include <QByteArrayMatcher>
#include <QVector>
#include "exif.h"
//#include "logfile.h"
#include "heicxmlreader.h"

HeicXmlReader::HeicXmlReader(const QByteArray &ba) {

    map.clear();

    //QByteArrayMatcher matchStart("<?xpacket begin=");
    //QByteArrayMatcher matchEnd("<?xpacket end=");

    QByteArrayMatcher matchStart("<x:xmpmeta");
    QByteArrayMatcher matchEnd("</x:xmpmeta");


    qsizetype start = matchStart.indexIn(ba);
    qsizetype offset = 0;
    qsizetype newOffset = offset+1;
    for (; offset < 0x200000 && newOffset > offset; ) {
        newOffset = matchEnd.indexIn(ba, offset+1);
        offset = newOffset;
        //LOGSTREAM << Qt::hex << offset;
    }
    qsizetype end = ba.indexOf(">", offset+1);
    if (-1 != start) {
        //LOGSTREAM << Qt::hex << start << end;
        //start = ba.indexOf(">", start+1)+2;
        //LOGSTREAM << Qt::hex << start << end;

        qsizetype length = end-start;
        QByteArray xmlAsBa = ba.sliced(start, length);
        QDomDocument xml;
        xml.setContent(xmlAsBa);

        extractFromDocument(xml);
    }

}



void HeicXmlReader::extractElements(const QDomNode &node, AttributesMap &result) {
    QDomNode child = node.firstChild();
    while (!child.isNull()) {
        if (child.isElement()) {
            QDomElement elem = child.toElement();
            // --- Extract element text ---
            QString text = elem.text().trimmed();
            if (!text.isEmpty()) {
                result[elem.tagName()] = text;
            }
            // --- Extract attributes ---
            QDomNamedNodeMap attrs = elem.attributes();
            for (int i = 0; i < attrs.count(); ++i) {
                QDomAttr attr = attrs.item(i).toAttr();
                if (!attr.value().isEmpty()) {
                    QString key = elem.tagName() + "@" + attr.name();
                    result[key] = attr.value();
                }
            }
            // Recurse into children
            extractElements(child, result);
        }
        child = child.nextSibling();
    }
}

std::string HeicXmlReader::cameraAsString() const {

    std::ostringstream cameraStream;
    cameraStream << map["tiff:Make"].toStdString();
    cameraStream << " ";
    cameraStream << map["tiff:Model"].toStdString();
    cameraStream << " ";
    cameraStream << map["exifEX:LensModel"].toStdString();
    easyexif::Rational fNumber = easyexif::Rational(map["exif:FNumber"]);
    cameraStream << fNumber.ratioF();
    cameraStream << " ";
    easyexif::Rational exposureTime = easyexif::Rational(map["exif:ExposureTime"]);
    cameraStream << exposureTime.ratio(" s");
    return cameraStream.str();
}

std::string HeicXmlReader::titleAsString() const {
    return map["dc:title"].toStdString();
}

std::string HeicXmlReader::linkLocation() const {
    return map["rdf:Description@Iptc4xmpCore:Location"].toStdString();
}

std::string HeicXmlReader::creatorAsString() const {
    return map["dc:creator"].toStdString();
}

std::string HeicXmlReader::descriptionAsString() const {
    return map["dc:description"].toStdString();
}


void HeicXmlReader::extractFromDocument(const QDomDocument &doc) {
    map.clear();
    extractElements(doc.documentElement(), map);
}
