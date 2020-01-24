#include "mapGraphicsShapeObject.h"
#include "commonConstants.h"
#include "basicMath.h"


#define MAPBORDER 10


MapGraphicsShapeObject::MapGraphicsShapeObject(MapGraphicsView* _view, MapGraphicsObject *parent) :
    MapGraphicsObject(true, parent)
{
    setFlag(MapGraphicsObject::ObjectIsSelectable, false);
    setFlag(MapGraphicsObject::ObjectIsMovable, false);
    setFlag(MapGraphicsObject::ObjectIsFocusable);
    view = _view;

    colorScale = new Crit3DColorScale();

    geoMap = new gis::Crit3DGeoMap();
    isDrawing = false;
    isFill = false;
    shapePointer = nullptr;
    nrShapes = 0;
    updateCenter();
}


/*!
\brief If sizeIsZoomInvariant() is true, this should return the size of the
 rectangle you want in PIXELS. If false, this should return the size of the rectangle in METERS. The
 rectangle should be centered at (0,0) regardless.
*/
QRectF MapGraphicsShapeObject::boundingRect() const
{
     return QRectF( -this->view->width() * 0.6, -this->view->height() * 0.6,
                     this->view->width() * 1.2,  this->view->height() * 1.2);
}


void MapGraphicsShapeObject::updateCenter()
{
    int widthPixels = view->width() - MAPBORDER*2;
    int heightPixels = view->height() - MAPBORDER*2;
    QPointF newCenter = view->mapToScene(QPoint(widthPixels/2, heightPixels/2));

    // reference point
    geoMap->referencePoint.latitude = newCenter.y();
    geoMap->referencePoint.longitude = newCenter.x();

    // reference pixel
    referencePixel = view->tileSource()->ll2qgs(newCenter, view->zoomLevel());

    if (isDrawing) setPos(newCenter);
}


void MapGraphicsShapeObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (this->isDrawing)
    {
        setMapExtents();

        if (this->shapePointer != nullptr)
            drawShape(painter);
    }
}


QPointF MapGraphicsShapeObject::getPixel(const LatLonPoint &geoPoint)
{
    QPointF point = QPointF(geoPoint.lon, geoPoint.lat);
    QPointF pixel = this->view->tileSource()->ll2qgs(point, this->view->zoomLevel());
    pixel.setX(pixel.x() - this->referencePixel.x());
    pixel.setY(this->referencePixel.y() - pixel.y());
    return pixel;
}


void MapGraphicsShapeObject::setPolygon(unsigned int i, unsigned int j, QPolygonF* polygon)
{
    QPointF point;

    polygon->clear();
    unsigned long offset = shapeParts[i][j].offset;
    unsigned long lenght = shapeParts[i][j].length;

    for (unsigned long v = 0; v < lenght; v++)
    {
        j = offset + v;
        point = getPixel(geoPoints[i][j]);
        polygon->append(point);
    }
}


void MapGraphicsShapeObject::drawShape(QPainter* myPainter)
{
    QPolygonF polygon;
    QPainterPath* path;
    QPainterPath* inner;

    myPainter->setPen(Qt::black);
    myPainter->setBrush(Qt::NoBrush);

    for (unsigned long i = 0; i < nrShapes; i++)
    {
        if (isFill)
        {
            if (values[i] != NODATA)
            {
                Crit3DColor* myColor = colorScale->getColor(values[i]);
                myPainter->setBrush(QColor(myColor->red, myColor->green, myColor->blue));
            }
            else
            {
                myPainter->setBrush(Qt::NoBrush);
            }
        }

        for (unsigned int j = 0; j < shapeParts[i].size(); j++)
        {
            if (shapeParts[i][j].hole)
                continue;

            if (geoBounds[i][j].v0.lon > geoMap->topRight.longitude
                    || geoBounds[i][j].v0.lat > geoMap->topRight.latitude
                    || geoBounds[i][j].v1.lon < geoMap->bottomLeft.longitude
                    || geoBounds[i][j].v1.lat < geoMap->bottomLeft.latitude)
            {
                continue;
            }

            setPolygon(i, j, &polygon);

            if (holes[i][j].size() == 0)
            {
                myPainter->drawPolygon(polygon);
            }
            else
            {
                path = new QPainterPath();
                inner = new QPainterPath();

                path->addPolygon(polygon);

                for (unsigned int k = 0; k < holes[i][j].size(); k++)
                {
                    setPolygon(i, holes[i][j][k], &polygon);
                    inner->addPolygon(polygon);
                }

                myPainter->drawPath(path->subtracted(*inner));

                delete inner;
                delete path;
            }
        }
    }
}


bool MapGraphicsShapeObject::initializeUTM(Crit3DShapeHandler* shapePtr)
{
    if (shapePtr == nullptr) return false;
    shapePointer = shapePtr;

    updateCenter();

    double lat, lon;
    ShapeObject myShape;
    Box<double>* bounds;
    const Point<double> *p_ptr;
    Point<double> point;

    nrShapes = unsigned(shapePointer->getShapeCount());
    shapeParts.resize(nrShapes);
    holes.resize(nrShapes);
    geoBounds.resize(nrShapes);
    geoPoints.resize(nrShapes);
    values.resize(nrShapes);

    double refLatitude = geoMap->referencePoint.latitude;

    int zoneNumber = shapePtr->getUtmZone();
    if (zoneNumber < 1 || zoneNumber > 60)
        return false;

    for (unsigned int i = 0; i < nrShapes; i++)
    {
        shapePointer->getShape(int(i), myShape);
        shapeParts[i] = myShape.getParts();

        // intialize values
        values[i] = NODATA;

        unsigned int nrParts = myShape.getPartCount();
        holes[i].resize(nrParts);
        geoBounds[i].resize(nrParts);

        for (unsigned int j = 0; j < nrParts; j++)
        {
            // bounds
            bounds = &(shapeParts[i][j].boundsPart);
            gis::utmToLatLon(zoneNumber, refLatitude, bounds->xmin, bounds->ymin, &lat, &lon);
            geoBounds[i][j].v0.lat = lat;
            geoBounds[i][j].v0.lon = lon;

            gis::utmToLatLon(zoneNumber, refLatitude, bounds->xmax, bounds->ymax, &lat, &lon);
            geoBounds[i][j].v1.lat = lat;
            geoBounds[i][j].v1.lon = lon;

            // holes
            if (shapeParts[i][j].hole)
            {
                // check first point
                unsigned long offset = shapeParts[i][j].offset;
                point = myShape.getVertex(offset);
                int index = myShape.getIndexPart(point.x, point.y);
                if (index != NODATA)
                {
                    holes[i][unsigned(index)].push_back(j);
                }
            }
        }

        // vertices
        unsigned long nrVertices = myShape.getVertexCount();
        geoPoints[i].resize(nrVertices);
        p_ptr = myShape.getVertices();
        for (unsigned long j = 0; j < nrVertices; j++)
        {
            gis::utmToLatLon(zoneNumber, refLatitude, p_ptr->x, p_ptr->y, &lat, &lon);
            geoPoints[i][j].lat = lat;
            geoPoints[i][j].lon = lon;
            p_ptr++;
        }
    }

    setDrawing(true);
    return true;
}


Crit3DShapeHandler* MapGraphicsShapeObject::getShapePointer()
{
    return this->shapePointer;
}


// warning: call after initializeUTM
void MapGraphicsShapeObject::setNumericValues(std::string fieldName)
{
    // set values
    float firstValue = NODATA;
    for (unsigned int i = 0; i < nrShapes; i++)
    {
        values[i] = float(shapePointer->getNumericValue(signed(i), fieldName));

        // TODO Fix problem with NULL
        if (isEqual(values[i], 0)) values[i] = NODATA;

        if (isEqual(firstValue, NODATA) && (! isEqual(values[i], NODATA)))
            firstValue = values[i];
    }

    // set min/max
    colorScale->minimum = firstValue;
    colorScale->maximum = firstValue;
    if (! isEqual(firstValue, NODATA))
    {
        for (unsigned int i = 0; i < nrShapes; i++)
            if (! isEqual(values[i], NODATA))
            {
                colorScale->minimum = MINVALUE(colorScale->minimum, values[i]);
                colorScale->maximum = MAXVALUE(colorScale->maximum, values[i]);
            }
    }
}


int MapGraphicsShapeObject::getCategoryIndex(std::string strValue)
{
    for (unsigned int i = 0; i < categories.size(); i++)
    {
        if (categories[i] == strValue) return signed(i);
    }
    return NODATA;
}


// warning: call after initializeUTM
void MapGraphicsShapeObject::setCategories(std::string fieldName)
{
    // fill categories and set values(index of categories)
    categories.clear();
    for (unsigned int i = 0; i < nrShapes; i++)
    {
        std::string strValue = shapePointer->getStringValue(signed(i), fieldName);

        if (strValue != "")
        {
            int index = getCategoryIndex(strValue);
            if (index != NODATA)
            {
                values[i] = index+1;
            }
            else
            {
                categories.push_back(strValue);
                values[i] = categories.size();
            }
        }
        else values[i] = NODATA;
    }

    // define min/max
    if (! categories.empty())
    {
        colorScale->minimum = 1;
        colorScale->maximum = categories.size();
    }
    else
    {
        colorScale->minimum = NODATA;
        colorScale->maximum = NODATA;
    }
}


void MapGraphicsShapeObject::setFill(bool value)
{
    isFill = value;
}


void MapGraphicsShapeObject::setDrawing(bool value)
{
    this->isDrawing = value;
}


void MapGraphicsShapeObject::setMapExtents()
{
    int widthPixels = view->width() - MAPBORDER*2;
    int heightPixels = view->height() - MAPBORDER*2;
    QPointF botLeft = view->mapToScene(QPoint(0, heightPixels));
    QPointF topRight = view->mapToScene(QPoint(widthPixels, 0));

    geoMap->bottomLeft.longitude = MAXVALUE(-180, botLeft.x());
    geoMap->bottomLeft.latitude = MAXVALUE(-84, botLeft.y());
    geoMap->topRight.longitude = MINVALUE(180, topRight.x());
    geoMap->topRight.latitude = MINVALUE(84, topRight.y());
}


void MapGraphicsShapeObject::clear()
{
    setDrawing(false);

    for (unsigned int i = 0; i < nrShapes; i++)
    {
        shapeParts[i].clear();
        holes[i].clear();
        geoBounds[i].clear();
        geoPoints[i].clear();
    }
    shapeParts.clear();
    holes.clear();
    geoBounds.clear();
    geoPoints.clear();

    nrShapes = 0;
}
