#ifndef LEGENDSDOM_H
#define LEGENDSDOM_H

#include <QDomDocument>
#include "rowtype.h"

class LegendsDom: public QDomDocument {
    typedef QDomDocument Super;
public:
    LegendsDom();

    void addLine(const RowType &completeLine);
    const RowType retrieveLine(const QDomNode &line) const;


private:
    QDomElement root;
    QDomElement line;

};

#endif // LEGENDSDOM_H
