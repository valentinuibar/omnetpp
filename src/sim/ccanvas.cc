//==========================================================================
//   CCANVAS.CC  -  part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2015 Andras Varga
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <map>
#include <algorithm>
#include "common/stringutil.h"
#include "common/colorutil.h"
#include "common/opp_ctype.h"
#include "omnetpp/ccanvas.h"
#include "omnetpp/cproperty.h"
#include "omnetpp/cproperties.h"
#include "omnetpp/csimulation.h"  // getEnvir()
#include "omnetpp/cstringtokenizer.h"
#include "omnetpp/platdep/platmisc.h"
#include "omnetpp/opp_string.h"

using namespace omnetpp::common;

namespace omnetpp {

using namespace canvas_stream_ops;

Register_Class(cGroupFigure);
//Register_Class(cPanelFigure);
Register_Class(cLineFigure);
Register_Class(cArcFigure);
Register_Class(cPolylineFigure);
Register_Class(cRectangleFigure);
Register_Class(cOvalFigure);
Register_Class(cRingFigure);
Register_Class(cPieSliceFigure);
Register_Class(cPolygonFigure);
Register_Class(cPathFigure);
Register_Class(cTextFigure);
Register_Class(cLabelFigure);
Register_Class(cImageFigure);
Register_Class(cIconFigure);
Register_Class(cPixmapFigure);

const cFigure::Color cFigure::BLACK(0, 0, 0);
const cFigure::Color cFigure::WHITE(255, 255, 255);
const cFigure::Color cFigure::GREY(128, 128, 128);
const cFigure::Color cFigure::RED(255, 0, 0);
const cFigure::Color cFigure::GREEN(0, 255, 0);
const cFigure::Color cFigure::BLUE(0, 0, 255);
const cFigure::Color cFigure::YELLOW(255, 255, 0);
const cFigure::Color cFigure::CYAN(0, 255, 255);
const cFigure::Color cFigure::MAGENTA(255, 0, 255);

const cFigure::Color cFigure::GOOD_DARK_COLORS[] = {
    cFigure::parseColor("darkBlue"),
    cFigure::parseColor("red2"),
    cFigure::parseColor("darkGreen"),
    cFigure::parseColor("orange"),
    cFigure::parseColor("#008000"),
    cFigure::parseColor("darkGrey"),
    cFigure::parseColor("#a00000"),
    cFigure::parseColor("#008080"),
    cFigure::parseColor("cyan"),
    cFigure::parseColor("#808000"),
    cFigure::parseColor("#8080ff"),
    cFigure::parseColor("yellow"),
    cFigure::parseColor("black"),
    cFigure::parseColor("purple"),
};

const cFigure::Color cFigure::GOOD_LIGHT_COLORS[] = {
    cFigure::parseColor("lightCyan"),
    cFigure::parseColor("lightCoral"),
    cFigure::parseColor("lightBlue"),
    cFigure::parseColor("lightGreen"),
    cFigure::parseColor("lightYellow"),
    cFigure::parseColor("plum1"),
    cFigure::parseColor("lightSalmon"),
    cFigure::parseColor("lightPink"),
    cFigure::parseColor("lightGrey"),
    cFigure::parseColor("mediumPurple"),
};

const int cFigure::NUM_GOOD_DARK_COLORS = sizeof(GOOD_DARK_COLORS) / sizeof(GOOD_DARK_COLORS[0]);
const int cFigure::NUM_GOOD_LIGHT_COLORS = sizeof(GOOD_LIGHT_COLORS) / sizeof(GOOD_LIGHT_COLORS[0]);

static const char *PKEY_TYPE = "type";
static const char *PKEY_VISIBLE = "visible";
static const char *PKEY_TAGS = "tags";
static const char *PKEY_TRANSFORM = "transform";
static const char *PKEY_CHILDZ = "childZ";
static const char *PKEY_BOUNDS = "bounds";
static const char *PKEY_POS = "pos";
static const char *PKEY_POINTS = "points";
static const char *PKEY_ANCHOR = "anchor";
static const char *PKEY_SIZE = "size";
static const char *PKEY_INNERSIZE = "innerSize";
static const char *PKEY_CORNERRADIUS = "cornerRadius";
static const char *PKEY_STARTANGLE = "startAngle";
static const char *PKEY_ENDANGLE = "endAngle";
static const char *PKEY_PATH = "path";
static const char *PKEY_SMOOTH = "smooth";
static const char *PKEY_OFFSET = "offset";
static const char *PKEY_LINECOLOR = "lineColor";
static const char *PKEY_LINESTYLE = "lineStyle";
static const char *PKEY_LINEWIDTH = "lineWidth";
static const char *PKEY_LINEOPACITY = "lineOpacity";
static const char *PKEY_CAPSTYLE = "capStyle";
static const char *PKEY_JOINSTYLE = "joinStyle";
static const char *PKEY_ZOOMLINEWIDTH = "zoomLineWidth";
static const char *PKEY_FILLCOLOR = "fillColor";
static const char *PKEY_FILLOPACITY = "fillOpacity";
static const char *PKEY_FILLRULE = "fillRule";
static const char *PKEY_STARTARROWHEAD = "startArrowhead";
static const char *PKEY_ENDARROWHEAD = "endArrowhead";
static const char *PKEY_TEXT = "text";
static const char *PKEY_FONT = "font";
static const char *PKEY_COLOR = "color";
static const char *PKEY_OPACITY = "opacity";
static const char *PKEY_IMAGE = "image";
static const char *PKEY_RESOLUTION = "resolution";
static const char *PKEY_INTERPOLATION = "interpolation";
static const char *PKEY_TINT = "tint";

int cFigure::lastId = 0;
cStringPool cFigure::stringPool;

#define ENSURE_RANGE01(var)  {if (var < 0 || var > 1) throw cRuntimeError(this, #var " must be in the range [0,1]");}
#define ENSURE_NONNEGATIVE(var)  {if (var < 0) throw cRuntimeError(this, #var " cannot be negative");}
#define ENSURE_POSITIVE(var)  {if (var <= 0) throw cRuntimeError(this, #var " cannot be negative or zero");}

std::string cFigure::Point::str() const
{
    std::stringstream os;
    os << "(" << x << ", " << y << ")";
    return os.str();
}

cFigure::Point cFigure::Point::operator+(const Point& p) const
{
    return Point(x + p.x, y + p.y);
}

cFigure::Point cFigure::Point::operator-(const cFigure::Point& p) const
{
    return Point(x - p.x, y - p.y);
}

cFigure::Point cFigure::Point::operator*(double s) const
{
    return Point(x * s, y * s);
}

cFigure::Point cFigure::Point::operator/(double s) const
{
    return Point(x / s, y / s);
}

double cFigure::Point::operator*(const Point& p) const
{
    return x * p.x + y * p.y;
}

double cFigure::Point::getLength() const
{
    return sqrt(x * x + y * y);
}

double cFigure::Point::distanceTo(const Point& p) const
{
    double dx = p.x - x;
    double dy = p.y - y;
    return sqrt(dx * dx + dy * dy);
}

std::string cFigure::Rectangle::str() const
{
    std::stringstream os;
    os << "(" << x << ", " << y << ", w=" << width << ", h=" << height << ")";
    return os.str();
}

cFigure::Point cFigure::Rectangle::getCenter() const
{
    return Point(x + width / 2.0, y + height / 2.0);
}

cFigure::Point cFigure::Rectangle::getSize() const
{
    return Point(width, height);
}

std::string cFigure::Color::str() const
{
    char buf[16];
    sprintf(buf, "#%2.2x%2.2x%2.2x", red, green, blue);
    return buf;
}

std::string cFigure::Font::str() const
{
    std::stringstream os;
    os << "(" << (typeface.empty() ? "<default>" : typeface) << ", ";
    os << (int)pointSize << "pt";
    if (style) {
        os << ",";
        if (style & cFigure::FONT_BOLD)  os << " bold";
        if (style & cFigure::FONT_ITALIC)  os << " italic";
        if (style & cFigure::FONT_UNDERLINE)  os << " underline";
    }
    os << ")";
    return os.str();
}

std::string cFigure::RGBA::str() const
{
    std::stringstream os;
    os << "(" << (int)red << ", " << (int)green << ", " << (int)blue << ", " << (int)alpha << ")";
    return os.str();
}

cFigure::Transform& cFigure::Transform::translate(double dx, double dy)
{
    t1 += dx;
    t2 += dy;
    return *this;
}

cFigure::Transform& cFigure::Transform::scale(double sx, double sy)
{
    *this = Transform(a*sx, b*sy, c*sx, d*sy, sx*t1, sy*t2);
    return *this;
}

cFigure::Transform& cFigure::Transform::scale(double sx, double sy, double cx, double cy)
{
    *this = Transform(a*sx, b*sy, c*sx, d*sy, sx*t1-cx*sx+cx, sy*t2-cy*sy+cy);
    return *this;
}

cFigure::Transform& cFigure::Transform::rotate(double phi)
{
    double cosPhi = cos(phi);
    double sinPhi = sin(phi);
    *this = Transform(
            a*cosPhi - b*sinPhi, a*sinPhi + b*cosPhi,
            c*cosPhi - d*sinPhi, c*sinPhi + d*cosPhi,
            t1*cosPhi - t2*sinPhi, t1*sinPhi + t2*cosPhi);
    return *this;
}

cFigure::Transform& cFigure::Transform::rotate(double phi, double cx, double cy)
{
    double cosPhi = cos(phi);
    double sinPhi = sin(phi);
    *this = Transform(
            a*cosPhi - b*sinPhi, a*sinPhi + b*cosPhi,
            c*cosPhi - d*sinPhi, c*sinPhi + d*cosPhi,
            -t2*sinPhi+cy*sinPhi+t1*cosPhi-cx*cosPhi+cx, t1*sinPhi-cx*sinPhi+t2*cosPhi-cy*cosPhi+cy);
    return *this;
}

cFigure::Transform& cFigure::Transform::skewx(double coeff)
{
    double a_ = b*coeff+a;
    double c_ = d*coeff+c;
    double t1_ = t2*coeff+t1;
    a = a_; c = c_; t1 = t1_;
    return *this;
}

cFigure::Transform& cFigure::Transform::skewy(double coeff)
{
    double b_ = a*coeff+b;
    double d_ = c*coeff+d;
    double t2_ = t1*coeff+t2;
    b = b_; d = d_; t2 = t2_;
    return *this;
}

cFigure::Transform& cFigure::Transform::skewx(double coeff, double cy)
{
    double a_ = b*coeff+a;
    double c_ = d*coeff+c;
    double t1_ = t2*coeff-cy*coeff+t1;
    a = a_; c = c_; t1 = t1_;
    return *this;
}

cFigure::Transform& cFigure::Transform::skewy(double coeff, double cx)
{
    double b_ = a*coeff+b;
    double d_ = c*coeff+d;
    double t2_ = t1*coeff-cx*coeff+t2;
    b = b_; d = d_; t2 = t2_;
    return *this;
}

cFigure::Transform& cFigure::Transform::multiply(const Transform& other)
{
    double a_ = a*other.a + b*other.c;
    double b_ = a*other.b + b*other.d;
    double c_ = c*other.a + d*other.c;
    double d_ = c*other.b + d*other.d;
    double t1_ = t1*other.a + t2*other.c + other.t1;
    double t2_ = t1*other.b + t2*other.d + other.t2;

    a = a_; b = b_;
    c = c_; d = d_;
    t1 = t1_; t2 = t2_;

    return *this;
}

cFigure::Transform& cFigure::Transform::rightMultiply(const Transform& other)
{
    double a_ = other.a*a + other.b*c;
    double b_ = other.a*b + other.b*d;
    double c_ = other.c*a + other.d*c;
    double d_ = other.c*b + other.d*d;
    double t1_ = other.t1*a + other.t2*c + t1;
    double t2_ = other.t1*b + other.t2*d + t2;

    a = a_; b = b_;
    c = c_; d = d_;
    t1 = t1_; t2 = t2_;

    return *this;
}


cFigure::Point cFigure::Transform::applyTo(const Point& p) const
{
    return Point(a*p.x + c*p.y + t1, b*p.x + d*p.y + t2);
}

std::string cFigure::Transform::str() const
{
    std::stringstream os;
    os << "((" << a << " " << b << ")";
    os << " (" << c << " " << d << ")";
    os << " (" << t1 << " " << t2 << "))";
    return os.str();
}

//----

cFigure::Pixmap::Pixmap() : width(0), height(0), data(nullptr)
{
}

cFigure::Pixmap::Pixmap(int width, int height) : width(width), height(height), data(nullptr)
{
    allocate(width, height);
    fill(BLACK, 0);
}

cFigure::Pixmap::Pixmap(int width, int height, const RGBA& fill_) : width(width), height(height), data(nullptr)
{
    allocate(width, height);
    fill(fill_);
}

cFigure::Pixmap::Pixmap(int width, int height, const Color& color, double opacity) : width(width), height(height), data(nullptr)
{
    allocate(width, height);
    fill(color, opacity);
}

cFigure::Pixmap::Pixmap(const Pixmap& other) : width(0), height(0), data(nullptr)
{
    *this = other;
}

cFigure::Pixmap::~Pixmap()
{
    delete[] data;
}

void cFigure::Pixmap::allocate(int newWidth, int newHeight)
{
    if (newWidth < 0 || newHeight < 0)
        throw cRuntimeError("cFigure::Pixmap: width/height cannot be negative");

    width = newWidth;
    height = newHeight;
    delete[] data;
    data = new RGBA[newWidth * newHeight];
}

void cFigure::Pixmap::setSize(int newWidth, int newHeight, const RGBA& fill_)
{
    if (newWidth == width && newHeight == height)
        return; // no change
    if (newWidth < 0 || newWidth < 0)
        throw cRuntimeError("cFigure::Pixmap: width/height cannot be negative");

    RGBA *newData = new RGBA[newWidth * newHeight];

    // pixel copying (could be more efficient)
    for (int y = 0; y < newHeight; y++)
        for (int x = 0; x < newWidth; x++)
            newData[y * newWidth + x] = (x < width && y < height) ? data[y * width + x] : fill_;


    width = newWidth;
    height = newHeight;
    delete[] data;
    data = newData;
}

void cFigure::Pixmap::setSize(int width, int height, const Color& color, double opacity)
{
    RGBA rgba(color.red, color.green, color.blue, alpha(opacity));
    setSize(width, height, rgba);
}

void cFigure::Pixmap::fill(const RGBA& rgba)
{
    int size = width * height;
    for (int i = 0; i < size; i++)
        data[i] = rgba;
}

void cFigure::Pixmap::fill(const Color& color, double opacity)
{
    RGBA rgba(color.red, color.green, color.blue, alpha(opacity));
    fill(rgba);
}

cFigure::Pixmap& cFigure::Pixmap::operator=(const Pixmap& other)
{
    if (width != other.width || height != other.height)
        allocate(other.width, other.height);
    memcpy(data, other.data, width * height * sizeof(RGBA));
    return *this;
}

cFigure::RGBA& cFigure::Pixmap::pixel(int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        throw cRuntimeError("cFigure::Pixmap: x or y coordinate out of bounds");
    return data[width * y + x];
}

std::string cFigure::Pixmap::str() const
{
    std::stringstream os;
    os << "(" << width << " x " << height << ")";
    return os.str();
}

//----

cFigure::cFigure(const char *name) : cOwnedObject(name), id(++lastId), visible(true),
        tags(nullptr), tagBits(0), localChanges(0), subtreeChanges(0)
{
}

cFigure::Point cFigure::parsePoint(cProperty *property, const char *key, int index)
{
    double x = opp_atof(opp_nulltoempty(property->getValue(key, index)));
    double y = opp_atof(opp_nulltoempty(property->getValue(key, index + 1)));
    return Point(x, y);
}

std::vector<cFigure::Point> cFigure::parsePoints(cProperty *property, const char *key)
{
    std::vector<Point> points;
    int n = property->getNumValues(key);
    if (n % 2 != 0)
        throw cRuntimeError("number of coordinates must be even");
    for (int i = 0; i < n; i += 2)
        points.push_back(parsePoint(property, key, i));
    return points;
}

cFigure::Rectangle cFigure::computeBoundingBox(const Point& position, const Point& size, double ascent, Anchor anchor)
{
    Point topleft;
    switch (anchor) {
        case cFigure::ANCHOR_CENTER:
            topleft.x = position.x - size.x / 2;
            topleft.y = position.y - size.y / 2;
            break;
        case cFigure::ANCHOR_N:
            topleft.x = position.x - size.x / 2;
            topleft.y = position.y;
            break;
        case cFigure::ANCHOR_E:
            topleft.x = position.x - size.x;
            topleft.y = position.y - size.y / 2;
            break;
        case cFigure::ANCHOR_S:
            topleft.x = position.x - size.x / 2;
            topleft.y = position.y - size.y;
            break;
        case cFigure::ANCHOR_W:
            topleft.x = position.x;
            topleft.y = position.y - size.y / 2;
            break;
        case cFigure::ANCHOR_NW:
            topleft.x = position.x;
            topleft.y = position.y;
            break;
        case cFigure::ANCHOR_NE:
            topleft.x = position.x - size.x;
            topleft.y = position.y;
            break;
        case cFigure::ANCHOR_SE:
            topleft.x = position.x - size.x;
            topleft.y = position.y - size.y;
            break;
        case cFigure::ANCHOR_SW:
            topleft.x = position.x;
            topleft.y = position.y - size.y;
            break;
        case cFigure::ANCHOR_BASELINE_START:
            topleft.x = position.x;
            topleft.y = position.y - ascent;
            break;
        case cFigure::ANCHOR_BASELINE_MIDDLE:
            topleft.x = position.x - size.x / 2;
            topleft.y = position.y - ascent;
            break;
        case cFigure::ANCHOR_BASELINE_END:
            topleft.x = position.x - size.x;
            topleft.y = position.y - ascent;
            break;
        default:
            throw cRuntimeError("Unexpected anchor %d", anchor);
    }
    return Rectangle(topleft.x, topleft.y, size.x, size.y);
}

cFigure::Rectangle cFigure::parseBounds(cProperty *property)
{
    if (property->containsKey(PKEY_BOUNDS)) {
        if (property->containsKey(PKEY_POS) || property->containsKey(PKEY_SIZE) || property->containsKey(PKEY_ANCHOR))
            throw cRuntimeError("%s, %s and %s are not allowed when %s is present", PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS);
        if (property->getNumValues(PKEY_BOUNDS) != 4)
            throw cRuntimeError("%s: x, y, width, height expected", PKEY_BOUNDS);
        Point p = parsePoint(property, PKEY_BOUNDS, 0);
        Point size = parsePoint(property, PKEY_BOUNDS, 2);
        return Rectangle(p.x, p.y, size.x, size.y);
    }
    else {
        int numCoords = property->getNumValues(PKEY_POS);
        if (numCoords != 2)
            throw cRuntimeError("%s: two coordinates expected", PKEY_POS);
        Point p = parsePoint(property, PKEY_POS, 0);
        Point size = parsePoint(property, PKEY_SIZE, 0);
        const char *anchorStr = property->getValue(PKEY_ANCHOR);
        Anchor anchor = opp_isblank(anchorStr) ? cFigure::ANCHOR_NW : parseAnchor(anchorStr);
        return computeBoundingBox(p, size, size.y, anchor);
    }
}

inline bool contains(const std::string& str, const std::string& substr)
{
    return str.find(substr) != std::string::npos;
}

inline double deg2rad(double deg) { return deg * M_PI / 180; }
inline double rad2deg(double rad) { return rad / M_PI * 180; }

cFigure::Transform cFigure::parseTransform(const char *s)
{
    std::string operation = opp_trim(opp_substringbefore(s, "(").c_str());
    if (operation == "") {
        cFigure::Transform t;
        int parsedChars = 0;
        int count = sscanf(s, " ( ( %lf %lf ) ( %lf %lf ) ( %lf %lf ) ) %n",
                &t.a, &t.b, &t.c, &t.d, &t.t1, &t.t2, &parsedChars);
        if (count != 6 || s[parsedChars] != 0)
            throw cRuntimeError("Invalid transform syntax, ((a b) (c d) (t1 t2)) or operation(...) expected");
        return t;
    }

    std::string rest = opp_trim(opp_substringafter(s, "(").c_str());
    bool missingRParen = !contains(rest, ")");
    std::string argsStr = opp_trim(opp_substringbefore(rest, ")").c_str());
    std::string trailingGarbage = opp_trim(opp_substringafter(rest, ")").c_str());
    if (operation == "" || missingRParen || trailingGarbage.size() != 0)
        throw cRuntimeError("Syntax error in '%s', 'operation(arg1,arg2...)' expected", s);
    std::vector<double> args = cStringTokenizer(argsStr.c_str(), ",").asDoubleVector();

    Transform transform;
    if (operation == "translate") {
        if (args.size() == 2)
            transform.translate(args[0], args[1]);
        else
            throw cRuntimeError("Wrong number of args in '%s', translate(dx,dxy) expected", s);
    }
    else if (operation == "rotate") {
        if (args.size() == 1)
            transform.rotate(deg2rad(args[0]));
        else if (args.size() == 3)
            transform.rotate(deg2rad(args[0]), args[1], args[2]);
        else
            throw cRuntimeError("Wrong number of args in '%s', rotate(deg) or rotate(deg,cx,cy) expected", s);
    }
    else if (operation == "scale") {
        if (args.size() == 1)
            transform.scale(args[0]);
        else if (args.size() == 2)
            transform.scale(args[0], args[1]);
        else if (args.size() == 3)
            transform.scale(args[0], args[0], args[1], args[2]);
        else if (args.size() == 4)
            transform.scale(args[0], args[1], args[2], args[3]);
        else
            throw cRuntimeError("Wrong number of args in '%s', scale(s), scale(sx,sy), scale(s,cx,cy), or scale(sx,sy,cx,cy) expected", s);
    }
    else if (operation == "skewx") {
        if (args.size() == 1)
            transform.skewx(args[0]);
        else if (args.size() == 2)
            transform.skewx(args[0], args[1]);
        else
            throw cRuntimeError("Wrong number of args in '%s', skewx(coeff) or skewx(coeff,cy) expected", s);
    }
    else if (operation == "skewy") {
        if (args.size() == 1)
            transform.skewy(args[0]);
        else if (args.size() == 2)
            transform.skewy(args[0], args[1]);
        else
            throw cRuntimeError("Wrong number of args in '%s', skewy(coeff) or skewy(coeff,cx) expected", s);
    }
    else if (operation == "matrix") {
        if (args.size() == 6)
            transform.multiply(Transform(args[0], args[1], args[2], args[3], args[4], args[5]));
        else
            throw cRuntimeError("Wrong number of args in '%s', matrix(a,b,c,d,t1,t2) expected", s);
    }
    else {
        throw cRuntimeError("Invalid operation %s, translate, rotate, scale, skewx, skewy or matrix expected: '%s'", operation.c_str(), s);
    }
    return transform;
}

cFigure::Transform cFigure::parseTransform(cProperty *property, const char *key)
{
    Transform transform;
    for (int index = 0; index < property->getNumValues(key); index++) {
        const char *step = property->getValue(key, index);
        transform.multiply(parseTransform(step));
    }
    return transform;
}

cFigure::Font cFigure::parseFont(cProperty *property, const char *key)
{
    const char *typeface = property->getValue(key, 0);
    int size = opp_atol(opp_nulltoempty(property->getValue(key, 1)));
    if (size <= 0)
        size = 0;  // default size
    int flags = 0;
    cStringTokenizer tokenizer(property->getValue(key, 2));
    while (tokenizer.hasMoreTokens()) {
        const char *token = tokenizer.nextToken();
        if (!strcmp(token, "normal"))  /*no-op*/;
        else if (!strcmp(token, "bold"))
            flags |= FONT_BOLD;
        else if (!strcmp(token, "italic"))
            flags |= FONT_ITALIC;
        else if (!strcmp(token, "underline"))
            flags |= FONT_UNDERLINE;
        else
            throw cRuntimeError("unknown font style '%s'", token);
    }
    return Font(opp_nulltoempty(typeface), size, flags);
}

cFigure::Point cFigure::parsePoint(const char *s)
{
    cFigure::Point p;
    int parsedChars = 0;
    int count = sscanf(s, " ( %lf , %lf ) %n", &p.x, &p.y, &parsedChars);
    if (count != 2 || s[parsedChars] != 0)
        throw cRuntimeError("Invalid point syntax, (x,y) expected");
    return p;
}

cFigure::Rectangle cFigure::parseRectangle(const char *s)
{
    cFigure::Rectangle r;
    int parsedChars = 0;
    int count = sscanf(s, " ( %lf , %lf , w = %lf , h = %lf ) %n", &r.x, &r.y, &r.width, &r.height, &parsedChars);
    if (count != 4 || s[parsedChars] != 0)
        throw cRuntimeError("Invalid rectangle syntax, (x,y,w=n,h=m) expected");
    return r;
}

cFigure::Font cFigure::parseFont(const char *s)
{
    // syntax: (<default>, 8pt, bold italic underline)
    cFigure::Font font;
    if (*s == '(') s++;
    const char *comma = strchr(s, ',');
    std::string typeface = comma ? std::string(s, comma-s) : s;
    typeface = opp_trim(typeface.c_str());
    if (typeface != "<default>")
        font.typeface = typeface;
    if (comma) {
        std::vector<std::string> items = cStringTokenizer(comma+1, "(), ").asVector();
        for (int i = 0; i < (int)items.size(); i++) {
            std::string item = opp_trim(items[i].c_str());
            if (opp_isdigit(item[0]))
                font.pointSize = strtol(item.c_str(), nullptr, 10);
            else if (item == "bold")
                font.style |= cFigure::FONT_BOLD;
            else if (item == "italic")
                font.style |= cFigure::FONT_ITALIC;
            else if (item == "underline")
                font.style |= cFigure::FONT_UNDERLINE;
            else
                throw cRuntimeError("Invalid font style '%s'", item.c_str());
        }
    }
    return font;
}

bool cFigure::parseBool(const char *s)
{
    if (!strcmp(s, "true"))
        return true;
    if (!strcmp(s, "false"))
        return false;
    throw cRuntimeError("invalid boolean value '%s'", s);
}

cFigure::Color cFigure::parseColor(const char *s)
{
    Color color;
    common::parseColor(s, color.red, color.green, color.blue);
    return color;
}

cFigure::LineStyle cFigure::parseLineStyle(const char *s)
{
    if (!strcmp(s,"solid")) return LINE_SOLID;
    if (!strcmp(s,"dotted")) return LINE_DOTTED;
    if (!strcmp(s,"dashed")) return LINE_DASHED;
    throw cRuntimeError("invalid line style '%s'", s);
}

cFigure::CapStyle cFigure::parseCapStyle(const char *s)
{
    if (!strcmp(s,"butt")) return CAP_BUTT;
    if (!strcmp(s,"square")) return CAP_SQUARE;
    if (!strcmp(s,"round")) return CAP_ROUND;
    throw cRuntimeError("invalid cap style '%s'", s);
}

cFigure::JoinStyle cFigure::parseJoinStyle(const char *s)
{
    if (!strcmp(s,"bevel")) return JOIN_BEVEL;
    if (!strcmp(s,"miter")) return JOIN_MITER;
    if (!strcmp(s,"round")) return JOIN_ROUND;
    throw cRuntimeError("invalid join style '%s'", s);
}

cFigure::FillRule cFigure::parseFillRule(const char *s)
{
    if (!strcmp(s,"evenodd")) return FILL_EVENODD;
    if (!strcmp(s,"nonzero")) return FILL_NONZERO;
    throw cRuntimeError("invalid fill rule '%s'", s);
}

cFigure::Arrowhead cFigure::parseArrowhead(const char *s)
{
    if (!strcmp(s,"none")) return ARROW_NONE;
    if (!strcmp(s,"simple")) return ARROW_SIMPLE;
    if (!strcmp(s,"triangle")) return ARROW_TRIANGLE;
    if (!strcmp(s,"barbed")) return ARROW_BARBED;
    throw cRuntimeError("invalid arrowhead style '%s'", s);
}

cFigure::Interpolation cFigure::parseInterpolation(const char *s)
{
    if (!strcmp(s,"none")) return INTERPOLATION_NONE;
    if (!strcmp(s,"fast")) return INTERPOLATION_FAST;
    if (!strcmp(s,"best")) return INTERPOLATION_BEST;
    throw cRuntimeError("invalid interpolation mode '%s'", s);
}


cFigure::Anchor cFigure::parseAnchor(const char *s)
{
    if (!strcmp(s,"c")) return ANCHOR_CENTER;
    if (!strcmp(s,"center")) return ANCHOR_CENTER;
    if (!strcmp(s,"n")) return ANCHOR_N;
    if (!strcmp(s,"e")) return ANCHOR_E;
    if (!strcmp(s,"s")) return ANCHOR_S;
    if (!strcmp(s,"w")) return ANCHOR_W;
    if (!strcmp(s,"nw")) return ANCHOR_NW;
    if (!strcmp(s,"ne")) return ANCHOR_NE;
    if (!strcmp(s,"se")) return ANCHOR_SE;
    if (!strcmp(s,"sw")) return ANCHOR_SW;
    if (!strcmp(s,"start")) return ANCHOR_BASELINE_START;
    if (!strcmp(s,"middle")) return ANCHOR_BASELINE_MIDDLE;
    if (!strcmp(s,"end")) return ANCHOR_BASELINE_END;
    throw cRuntimeError("invalid anchor '%s'", s);
}

cFigure::~cFigure()
{
    for (int i = 0; i < (int)children.size(); i++)
        dropAndDelete(children[i]);
    stringPool.release(tags);
}

void cFigure::parse(cProperty *property)
{
    validatePropertyKeys(property);

    const char *s;
    if ((s = property->getValue(PKEY_VISIBLE)) != nullptr)
        setVisible(parseBool(s));
    int numTags = property->getNumValues(PKEY_TAGS);
    if (numTags > 0) {
        std::string tags;
        for (int i = 0; i < numTags; i++)
            tags += std::string(" ") + property->getValue(PKEY_TAGS, i);
        setTags(tags.c_str());
    }
    if (property->getNumValues(PKEY_TRANSFORM) != 0)
        setTransform(parseTransform(property, PKEY_TRANSFORM));
}

void cFigure::validatePropertyKeys(cProperty *property) const
{
    const std::vector<const char *>& keys = property->getKeys();
    for (int i = 0; i < (int)keys.size(); i++) {
        const char *key = keys[i];
        if (!isAllowedPropertyKey(key)) {
            std::string allowedList = opp_join(getAllowedPropertyKeys(), ", ");
            throw cRuntimeError(this, "Unknown property key '%s'; supported keys are: %s, and any string starting with 'x-'",
                    key, allowedList.c_str());
        }
    }
}

bool cFigure::isAllowedPropertyKey(const char *key) const
{
    const char **allowedKeys = getAllowedPropertyKeys();
    for (const char **allowedKeyPtr = allowedKeys; *allowedKeyPtr; allowedKeyPtr++)
        if (strcmp(key, *allowedKeyPtr) == 0)
            return true;
    if (key[0] == 'x' && key[1] == '-')  // "x-*" keys can be added by the user
        return true;
    return false;
}

const char **cFigure::getAllowedPropertyKeys() const
{
    static const char *keys[] = { PKEY_TYPE, PKEY_VISIBLE, PKEY_TAGS, PKEY_CHILDZ, PKEY_TRANSFORM, nullptr};
    return keys;
}

void cFigure::concatArrays(const char **dest, const char **first, const char **second)
{
    while (*first)
        *dest++ = *first++;
    while (*second)
        *dest++ = *second++;
    *dest = nullptr;
}

cFigure *cFigure::dupTree() const
{
    cFigure *result = dup();
    for (int i = 0; i < (int)children.size(); i++)
        result->addFigure(children[i]->dupTree());
    return result;
}

void cFigure::copy(const cFigure& other)
{
    setVisible(other.isVisible());
    setTags(other.getTags());
    setTransform(other.getTransform());
}

cFigure& cFigure::operator=(const cFigure& other)
{
    if (this == &other)
        return *this;
    cOwnedObject::operator=(other);
    copy(other);
    return *this;
}

void cFigure::forEachChild(cVisitor *v)
{
    for (int i = 0; i < (int)children.size(); i++)
        v->visit(children[i]);
}

std::string cFigure::info() const
{
    return "";
}

void cFigure::setTags(const char *tags)
{
    if (omnetpp::opp_strcmp(this->tags, tags) == 0)
        return;
    const char *oldTags = this->tags;
    this->tags = stringPool.get(tags);
    stringPool.release(oldTags);
    refreshTagBits();
    fire(CHANGE_TAGS);
}

void cFigure::addFigure(cFigure *figure)
{
    if (!figure)
        throw cRuntimeError(this, "addFigure(): cannot insert nullptr");
    take(figure);
    children.push_back(figure);
    refreshTagBits();
    figure->clearChangeFlags();
    fireStructuralChange();
}

void cFigure::addFigure(cFigure *figure, int pos)
{
    if (!figure)
        throw cRuntimeError(this, "addFigure(): cannot insert nullptr");
    if (pos < 0 || pos > (int)children.size())
        throw cRuntimeError(this, "addFigure(): insert position %d out of bounds", pos);
    take(figure);
    children.insert(children.begin() + pos, figure);
    refreshTagBits();
    figure->clearChangeFlags();
    fireStructuralChange();
}

void cFigure::addFigureAbove(cFigure *figure, cFigure *referenceFigure)
{
    int refPos = findFigure(referenceFigure);
    if (refPos == -1)
        throw cRuntimeError(this, "addFigureAbove(): reference figure is not a child");
    addFigure(figure, refPos + 1);
}

void cFigure::addFigureBelow(cFigure *figure, cFigure *referenceFigure)
{
    int refPos = findFigure(referenceFigure);
    if (refPos == -1)
        throw cRuntimeError(this, "addFigureBelow(): reference figure is not a child");
    addFigure(figure, refPos);
}

inline double getZ(cFigure *figure, const std::map<cFigure *, double>& orderMap)
{
    const double defaultZ = 0.0;
    std::map<cFigure *, double>::const_iterator it = orderMap.find(figure);
    return (it == orderMap.end()) ? defaultZ : it->second;
}

struct LessZ
{
    std::map<cFigure *, double>& orderMap;
    LessZ(std::map<cFigure *, double>& orderMap) : orderMap(orderMap) {}
    bool operator()(cFigure *figure1, cFigure *figure2) { return getZ(figure1, orderMap) < getZ(figure2, orderMap); }
};

void cFigure::insertChild(cFigure *figure, std::map<cFigure *, double>& orderMap)
{
    // Assuming that existing children are z-ordered, insert a new child at the appropriate place.
    // Z-order comes from the orderMap; if a figure is not in the map, its Z is assumed to be zero.
    take(figure);
    std::vector<cFigure *>::iterator it = std::upper_bound(children.begin(), children.end(), figure, LessZ(orderMap));
    children.insert(it, figure);
    refreshTagBits();
    fireStructuralChange();
}

cFigure *cFigure::removeFromParent()
{
    cFigure *parent = getParentFigure();
    return !parent ? this : parent->removeFigure(this);
}

cFigure *cFigure::removeFigure(int pos)
{
    if (pos < 0 || pos >= (int)children.size())
        throw cRuntimeError(this, "removeFigure(): index %d out of bounds", pos);
    cFigure *figure = children[pos];
    children.erase(children.begin() + pos);
    drop(figure);
    fireStructuralChange();
    return figure;
}

cFigure *cFigure::removeFigure(cFigure *figure)
{
    int pos = findFigure(figure);
    if (pos == -1)
        throw cRuntimeError(this, "removeFigure(): figure is not a child");
    return removeFigure(pos);
}

int cFigure::findFigure(const char *name) const
{
    for (int i = 0; i < (int)children.size(); i++)
        if (children[i]->isName(name))
            return i;
    return -1;
}

int cFigure::findFigure(cFigure *figure) const
{
    for (int i = 0; i < (int)children.size(); i++)
        if (children[i] == figure)
            return i;
    return -1;
}

cFigure *cFigure::getFigure(int pos) const
{
    if (pos < 0 || pos >= (int)children.size())
        throw cRuntimeError(this, "getFigure(): index %d out of bounds", pos);
    return children[pos];
}

cFigure *cFigure::getFigure(const char *name) const
{
    for (int i = 0; i < (int)children.size(); i++) {
        cFigure *figure = children[i];
        if (figure->isName(name))
            return figure;
    }
    return nullptr;
}

cFigure *cFigure::findFigureRecursively(const char *name) const
{
    if (!strcmp(name, getFullName()))
        return const_cast<cFigure *>(this);
    for (int i = 0; i < (int)children.size(); i++) {
        cFigure *figure = children[i]->findFigureRecursively(name);
        if (figure)
            return figure;
    }
    return nullptr;
}

cCanvas *cFigure::getCanvas() const
{
    return dynamic_cast<cCanvas *>(getRootFigure()->getOwner());
}

static char *nextToken(char *& rest)
{
    if (!rest)
        return nullptr;
    char *token = rest;
    rest = strchr(rest, '.');
    if (rest)
        *rest++ = '\0';
    return token;
}

cFigure *cFigure::getRootFigure() const
{
    cFigure *figure = const_cast<cFigure *>(this);
    cFigure *parent;
    while ((parent = figure->getParentFigure()) != nullptr)
        figure = parent;
    return figure;
}

cFigure *cFigure::getFigureByPath(const char *path) const
{
    if (!path || !path[0])
        return nullptr;

    // determine starting point
    bool isRelative = (path[0] == '.' || path[0] == '^');
    cFigure *rootFigure = isRelative ? nullptr : getRootFigure();  // only needed when processing absolute paths
    cFigure *figure = isRelative ? const_cast<cFigure *>(this) : rootFigure;
    if (path[0] == '.')
        path++;  // skip initial dot

    // match components of the path
    opp_string pathbuf(path);
    char *rest = pathbuf.buffer();
    char *token = nextToken(rest);
    while (token && figure) {
        if (!token[0])
            ;  /*skip empty path component*/
        else if (token[0] == '^' && token[1] == '\0')
            figure = figure->getParentFigure();
        else
            figure = figure->getFigure(token);
        token = nextToken(rest);
    }
    return figure;  // nullptr if not found
}

void cFigure::refreshTagBits()
{
    cCanvas *canvas = getCanvas();
    if (canvas) {
        tagBits = canvas->parseTags(getTags());
        for (int i = 0; i < (int)children.size(); i++)
            children[i]->refreshTagBits();
    }
}

void cFigure::fire(uint8_t flags)
{
    if ((localChanges & flags) == 0) {  // not yet set
        localChanges |= flags;
        for (cFigure *figure = getParentFigure(); figure != nullptr; figure = figure->getParentFigure())
            figure->subtreeChanges |= flags;
    }
}

void cFigure::clearChangeFlags()
{
    if (subtreeChanges)
        for (int i = 0; i < (int)children.size(); i++)
            children[i]->clearChangeFlags();
    localChanges = subtreeChanges = 0;
}

void cFigure::raiseAbove(cFigure *figure)
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "raiseAbove(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    int refPos = parent->findFigure(figure);
    if (refPos == -1)
        throw cRuntimeError(this, "raiseAbove(): reference figure must have the same parent");
    if (myPos < refPos) {
        parent->children.erase(parent->children.begin() + myPos);  // note: reference figure will be shifted down
        parent->children.insert(parent->children.begin() + refPos, this);
        fireStructuralChange();
    }
}

void cFigure::lowerBelow(cFigure *figure)
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "lowerBelow(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    int refPos = parent->findFigure(figure);
    if (refPos == -1)
        throw cRuntimeError(this, "lowerBelow(): reference figure must have the same parent");
    if (myPos > refPos) {
        parent->children.erase(parent->children.begin() + myPos);
        parent->children.insert(parent->children.begin() + refPos, this);
        fireStructuralChange();
    }
}

void cFigure::raiseToTop()
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "raiseToTop(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    if (myPos != (int)parent->children.size() - 1) {
        parent->children.erase(parent->children.begin() + myPos);
        parent->children.push_back(this);
        fireStructuralChange();
    }
}

void cFigure::lowerToBottom()
{
    cFigure *parent = getParentFigure();
    if (!parent)
        throw cRuntimeError(this, "lowerToBottom(): figure has no parent figure");
    int myPos = parent->findFigure(this);
    if (myPos != 0) {
        parent->children.erase(parent->children.begin() + myPos);
        parent->children.insert(parent->children.begin(), this);
        fireStructuralChange();
    }
}

void cFigure::move(double dx, double dy)
{
    moveLocal(dx, dy);
    for (int i = 0; i < (int)children.size(); i++)
        children[i]->move(dx, dy);
}

//----

cGroupFigure& cGroupFigure::operator=(const cGroupFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cGroupFigure::info() const
{
    return "";
}

//----

#if 0
void cPanelFigure::copy(const cPanelFigure& other)
{
    setPosition(other.getPosition());
}

cPanelFigure& cPanelFigure::operator=(const cPanelFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPanelFigure::info() const
{
    std::stringstream os;
    os << "at " << getPosition();
    return os.str();
}

void cPanelFigure::parse(cProperty *property)
{
    cFigure::parse(property);
    setPosition(parsePoint(property, PKEY_POS, 0));
}

const char **cPanelFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPanelFigure::updateParentTransform(Transform& transform)
{
    // replace current transform with an axis-aligned, unscaled (thus also unzoomable)
    // coordinate system, with the origin at getPosition()
    Point origin = transform.applyTo(getPosition());
    transform = Transform().translate(origin.x, origin.y);

    // then apply our own transform in the normal way (like all other figures do)
    transform.rightMultiply(getTransform());
}
#endif

//----

void cAbstractLineFigure::copy(const cAbstractLineFigure& other)
{
    setLineColor(other.getLineColor());
    setLineStyle(other.getLineStyle());
    setLineWidth(other.getLineWidth());
    setLineOpacity(other.getLineOpacity());
    setCapStyle(other.getCapStyle());
    setStartArrowhead(other.getStartArrowhead());
    setEndArrowhead(other.getEndArrowhead());
    setZoomLineWidth(other.getZoomLineWidth());
}

cAbstractLineFigure& cAbstractLineFigure::operator=(const cAbstractLineFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cAbstractLineFigure::info() const
{
    return "";
}

void cAbstractLineFigure::parse(cProperty *property)
{
    cFigure::parse(property);
    const char *s;
    if ((s = property->getValue(PKEY_LINECOLOR)) != nullptr)
        setLineColor(parseColor(s));
    if ((s = property->getValue(PKEY_LINESTYLE)) != nullptr)
        setLineStyle(parseLineStyle(s));
    if ((s = property->getValue(PKEY_LINEWIDTH)) != nullptr)
        setLineWidth(opp_atof(s));
    if ((s = property->getValue(PKEY_LINEOPACITY)) != nullptr)
        setLineOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_CAPSTYLE, 0)) != nullptr)
        setCapStyle(parseCapStyle(s));
    if ((s = property->getValue(PKEY_STARTARROWHEAD)) != nullptr)
        setStartArrowhead(parseArrowhead(s));
    if ((s = property->getValue(PKEY_ENDARROWHEAD)) != nullptr)
        setEndArrowhead(parseArrowhead(s));
    if ((s = property->getValue(PKEY_ZOOMLINEWIDTH)) != nullptr)
        setZoomLineWidth(parseBool(s));
}

const char **cAbstractLineFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_LINECOLOR, PKEY_LINESTYLE, PKEY_LINEWIDTH, PKEY_LINEOPACITY, PKEY_CAPSTYLE, PKEY_STARTARROWHEAD, PKEY_ENDARROWHEAD, PKEY_ZOOMLINEWIDTH, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractLineFigure::setLineColor(const Color& lineColor)
{
    if (lineColor == this->lineColor)
        return;
    this->lineColor = lineColor;
    fireVisualChange();
}

void cAbstractLineFigure::setLineWidth(double lineWidth)
{
    if (lineWidth == this->lineWidth)
        return;
    ENSURE_POSITIVE(lineWidth);
    this->lineWidth = lineWidth;
    fireVisualChange();
}

void cAbstractLineFigure::setLineOpacity(double lineOpacity)
{
    if (lineOpacity == this->lineOpacity)
        return;
    ENSURE_RANGE01(lineOpacity);
    this->lineOpacity = lineOpacity;
    fireVisualChange();
}

void cAbstractLineFigure::setLineStyle(LineStyle lineStyle)
{
    if (lineStyle == this->lineStyle)
        return;
    this->lineStyle = lineStyle;
    fireVisualChange();
}

void cAbstractLineFigure::setCapStyle(CapStyle capStyle)
{
    if (capStyle == this->capStyle)
        return;
    this->capStyle = capStyle;
    fireVisualChange();
}

void cAbstractLineFigure::setStartArrowhead(Arrowhead startArrowhead)
{
    if (startArrowhead == this->startArrowhead)
        return;
    this->startArrowhead = startArrowhead;
    fireVisualChange();
}

void cAbstractLineFigure::setEndArrowhead(Arrowhead endArrowhead)
{
    if (endArrowhead == this->endArrowhead)
        return;
    this->endArrowhead = endArrowhead;
    fireVisualChange();
}

void cAbstractLineFigure::setZoomLineWidth(bool zoomLineWidth)
{
    if (zoomLineWidth == this->zoomLineWidth)
        return;
    this->zoomLineWidth = zoomLineWidth;
    fireVisualChange();
}

//----

void cLineFigure::copy(const cLineFigure& other)
{
    setStart(other.getStart());
    setEnd(other.getEnd());
}

cLineFigure& cLineFigure::operator=(const cLineFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractLineFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cLineFigure::info() const
{
    std::stringstream os;
    os << getStart() << " to " << getEnd();
    return os.str();
}

void cLineFigure::parse(cProperty *property)
{
    cAbstractLineFigure::parse(property);

    setStart(parsePoint(property, PKEY_POINTS, 0));
    setEnd(parsePoint(property, PKEY_POINTS, 2));
}

const char **cLineFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POINTS, nullptr};
        concatArrays(keys, cAbstractLineFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cLineFigure::moveLocal(double dx, double dy)
{
    start.x += dx;
    start.y += dy;
    end.x += dx;
    end.y += dy;
    fireGeometryChange();
}

void cLineFigure::setStart(const Point& start)
{
    if (start == this->start)
        return;
    this->start = start;
    fireGeometryChange();
}

void cLineFigure::setEnd(const Point& end)
{
    if (end == this->end)
        return;
    this->end = end;
    fireGeometryChange();
}

//----

void cArcFigure::copy(const cArcFigure& other)
{
    setBounds(other.getBounds());
    setStartAngle(other.getStartAngle());
    setEndAngle(other.getEndAngle());
}

cArcFigure& cArcFigure::operator=(const cArcFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractLineFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cArcFigure::info() const
{
    std::stringstream os;
    os << getBounds() << ", " << floor(rad2deg(getStartAngle())) << " to " << floor(rad2deg(getEndAngle())) << " degrees";
    return os.str();
}

void cArcFigure::parse(cProperty *property)
{
    cAbstractLineFigure::parse(property);

    setBounds(parseBounds(property));

    const char *s;
    if ((s = property->getValue(PKEY_STARTANGLE)) != nullptr)
        setStartAngle(deg2rad(opp_atof(s)));
    if ((s = property->getValue(PKEY_ENDANGLE)) != nullptr)
        setEndAngle(deg2rad(opp_atof(s)));
}

const char **cArcFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_BOUNDS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_STARTANGLE, PKEY_ENDANGLE, nullptr};
        concatArrays(keys, cAbstractLineFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cArcFigure::moveLocal(double dx, double dy)
{
    bounds.x += dx;
    bounds.y += dy;
    fireGeometryChange();
}

void cArcFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cArcFigure::setStartAngle(double startAngle)
{
    if (startAngle == this->startAngle)
        return;
    this->startAngle = startAngle;
    fireGeometryChange();
}

void cArcFigure::setEndAngle(double endAngle)
{
    if (endAngle == this->endAngle)
        return;
    this->endAngle = endAngle;
    fireGeometryChange();
}

//----

void cPolylineFigure::copy(const cPolylineFigure& other)
{
    setPoints(other.getPoints());
    setSmooth(other.getSmooth());
    setJoinStyle(other.getJoinStyle());
}

void cPolylineFigure::checkIndex(int i) const
{
    if (i < 0 || i >= (int)points.size())
        throw cRuntimeError(this, "Index %d is out of range", i);
}

void cPolylineFigure::checkInsIndex(int i) const
{
    if (i < 0 || i > (int)points.size())
        throw cRuntimeError(this, "Insertion index %d is out of range", i);
}

cPolylineFigure& cPolylineFigure::operator=(const cPolylineFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractLineFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPolylineFigure::info() const
{
    std::stringstream os;
    for (int i = 0; i < (int)points.size(); i++)
        os << points[i] << " ";
    return os.str();
}

void cPolylineFigure::parse(cProperty *property)
{
    cAbstractLineFigure::parse(property);

    const char *s;
    setPoints(parsePoints(property, PKEY_POINTS));
    if ((s = property->getValue(PKEY_SMOOTH, 0)) != nullptr)
        setSmooth(parseBool(s));
    if ((s = property->getValue(PKEY_JOINSTYLE, 0)) != nullptr)
        setJoinStyle(parseJoinStyle(s));
}

const char **cPolylineFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POINTS, PKEY_SMOOTH, PKEY_JOINSTYLE, nullptr};
        concatArrays(keys, cAbstractLineFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPolylineFigure::moveLocal(double dx, double dy)
{
    for (int i = 0; i < (int)points.size(); i++) {
        points[i].x += dx;
        points[i].y += dy;
    }
    fireGeometryChange();
}

void cPolylineFigure::setPoints(const std::vector<Point>& points)
{
    if (points == this->points)
        return;
    this->points = points;
    fireGeometryChange();
}

void cPolylineFigure::setPoint(int i, const Point& point)
{
    checkIndex(i);
    if (point == this->points[i])
        return;
    this->points[i] = point;
    fireGeometryChange();
}

void cPolylineFigure::addPoint(const Point& point)
{
    this->points.push_back(point);
    fireGeometryChange();
}

void cPolylineFigure::removePoint(int i)
{
    checkIndex(i);
    this->points.erase(this->points.begin() + i);
    fireGeometryChange();
}

void cPolylineFigure::insertPoint(int i, const Point& point)
{
    checkInsIndex(i);
    this->points.insert(this->points.begin() + i, point);
    fireGeometryChange();
}

void cPolylineFigure::setSmooth(bool smooth)
{
    if (smooth == this->smooth)
        return;
    this->smooth = smooth;
    fireVisualChange();
}

void cPolylineFigure::setJoinStyle(JoinStyle joinStyle)
{
    if (joinStyle == this->joinStyle)
        return;
    this->joinStyle = joinStyle;
    fireVisualChange();
}

//----

void cAbstractShapeFigure::copy(const cAbstractShapeFigure& other)
{
    setOutlined(other.isOutlined());
    setFilled(other.isFilled());
    setLineColor(other.getLineColor());
    setFillColor(other.getFillColor());
    setLineStyle(other.getLineStyle());
    setLineWidth(other.getLineWidth());
    setLineOpacity(other.getLineOpacity());
    setFillOpacity(other.getFillOpacity());
    setZoomLineWidth(other.getZoomLineWidth());
}

cAbstractShapeFigure& cAbstractShapeFigure::operator=(const cAbstractShapeFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cAbstractShapeFigure::info() const
{
    return "";
}

void cAbstractShapeFigure::parse(cProperty *property)
{
    cFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_LINECOLOR)) != nullptr)
        setLineColor(parseColor(s));
    else if (property->containsKey(PKEY_LINECOLOR)) // but it's empty
        setOutlined(false);
    if ((s = property->getValue(PKEY_FILLCOLOR)) != nullptr) {
        setFillColor(parseColor(s));
        setFilled(true);
    }
    if ((s = property->getValue(PKEY_LINESTYLE)) != nullptr)
        setLineStyle(parseLineStyle(s));
    if ((s = property->getValue(PKEY_LINEWIDTH)) != nullptr)
        setLineWidth(opp_atof(s));
    if ((s = property->getValue(PKEY_LINEOPACITY)) != nullptr)
        setLineOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_FILLOPACITY)) != nullptr)
        setFillOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_ZOOMLINEWIDTH)) != nullptr)
        setZoomLineWidth(parseBool(s));
}

const char **cAbstractShapeFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_LINECOLOR, PKEY_FILLCOLOR, PKEY_LINESTYLE, PKEY_LINEWIDTH, PKEY_LINEOPACITY, PKEY_FILLOPACITY, PKEY_ZOOMLINEWIDTH, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractShapeFigure::setFilled(bool filled)
{
    if (filled == this->filled)
        return;
    this->filled = filled;
    fireVisualChange();
}

void cAbstractShapeFigure::setOutlined(bool outlined)
{
    if (outlined == this->outlined)
        return;
    this->outlined = outlined;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineColor(const Color& lineColor)
{
    if (lineColor == this->lineColor)
        return;
    this->lineColor = lineColor;
    fireVisualChange();
}

void cAbstractShapeFigure::setFillColor(const Color& fillColor)
{
    if (fillColor == this->fillColor)
        return;
    this->fillColor = fillColor;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineStyle(LineStyle lineStyle)
{
    if (lineStyle == this->lineStyle)
        return;
    this->lineStyle = lineStyle;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineWidth(double lineWidth)
{
    if (lineWidth == this->lineWidth)
        return;
    ENSURE_POSITIVE(lineWidth);
    this->lineWidth = lineWidth;
    fireVisualChange();
}

void cAbstractShapeFigure::setLineOpacity(double lineOpacity)
{
    if (lineOpacity == this->lineOpacity)
        return;
    ENSURE_RANGE01(lineOpacity);
    this->lineOpacity = lineOpacity;
    fireVisualChange();
}

void cAbstractShapeFigure::setFillOpacity(double fillOpacity)
{
    if (fillOpacity == this->fillOpacity)
        return;
    ENSURE_RANGE01(fillOpacity);
    this->fillOpacity = fillOpacity;
    fireVisualChange();
}

void cAbstractShapeFigure::setZoomLineWidth(bool zoomLineWidth)
{
    if (zoomLineWidth == this->zoomLineWidth)
        return;
    this->zoomLineWidth = zoomLineWidth;
    fireVisualChange();
}

//----

void cRectangleFigure::copy(const cRectangleFigure& other)
{
    setBounds(other.getBounds());
    setCornerRx(other.getCornerRx());
    setCornerRy(other.getCornerRy());
}

cRectangleFigure& cRectangleFigure::operator=(const cRectangleFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cRectangleFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cRectangleFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));
    const char *s;
    if ((s = property->getValue(PKEY_CORNERRADIUS, 0)) != nullptr) {
        setCornerRx(opp_atof(s));
        setCornerRy(opp_atof(s));  // as default
    }
    if ((s = property->getValue(PKEY_CORNERRADIUS, 1)) != nullptr)
        setCornerRy(opp_atof(s));
}

const char **cRectangleFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_BOUNDS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_CORNERRADIUS, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cRectangleFigure::moveLocal(double dx, double dy)
{
    bounds.x += dx;
    bounds.y += dy;
    fireGeometryChange();
}

void cRectangleFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cRectangleFigure::setCornerRx(double cornerRx)
{
    if (cornerRx == this->cornerRx)
        return;
    ENSURE_NONNEGATIVE(cornerRx);
    this->cornerRx = cornerRx;
    fireGeometryChange();
}

void cRectangleFigure::setCornerRy(double cornerRy)
{
    if (cornerRy == this->cornerRy)
        return;
    ENSURE_NONNEGATIVE(cornerRy);
    this->cornerRy = cornerRy;
    fireGeometryChange();
}

//----

void cOvalFigure::copy(const cOvalFigure& other)
{
    setBounds(other.getBounds());
}

cOvalFigure& cOvalFigure::operator=(const cOvalFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cOvalFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cOvalFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));
}

const char **cOvalFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_BOUNDS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cOvalFigure::moveLocal(double dx, double dy)
{
    bounds.x += dx;
    bounds.y += dy;
    fireGeometryChange();
}

void cOvalFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

//----

void cRingFigure::copy(const cRingFigure& other)
{
    setBounds(other.getBounds());
    setInnerRx(other.getInnerRx());
    setInnerRy(other.getInnerRy());
}

cRingFigure& cRingFigure::operator=(const cRingFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cRingFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cRingFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));
    const char *s;
    if ((s = property->getValue(PKEY_INNERSIZE, 0)) != nullptr) {
        setInnerRx(opp_atof(s) / 2);
        setInnerRy(opp_atof(s) / 2);  // as default
    }
    if ((s = property->getValue(PKEY_INNERSIZE, 1)) != nullptr)
        setInnerRy(opp_atof(s) / 2);
}

const char **cRingFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_BOUNDS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_INNERSIZE, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cRingFigure::moveLocal(double dx, double dy)
{
    bounds.x += dx;
    bounds.y += dy;
    fireGeometryChange();
}

void cRingFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cRingFigure::setInnerRx(double innerRx)
{
    if (innerRx == this->innerRx)
        return;
    ENSURE_NONNEGATIVE(innerRx);
    this->innerRx = innerRx;
    fireGeometryChange();
}

void cRingFigure::setInnerRy(double innerRy)
{
    if (innerRy == this->innerRy)
        return;
    ENSURE_NONNEGATIVE(innerRy);
    this->innerRy = innerRy;
    fireGeometryChange();
}

//----

void cPieSliceFigure::copy(const cPieSliceFigure& other)
{
    setBounds(other.getBounds());
    setStartAngle(other.getStartAngle());
    setEndAngle(other.getEndAngle());
}

cPieSliceFigure& cPieSliceFigure::operator=(const cPieSliceFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPieSliceFigure::info() const
{
    std::stringstream os;
    os << getBounds();
    return os.str();
}

void cPieSliceFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    setBounds(parseBounds(property));

    const char *s;
    if ((s = property->getValue(PKEY_STARTANGLE)) != nullptr)
        setStartAngle(deg2rad(opp_atof(s)));
    if ((s = property->getValue(PKEY_ENDANGLE)) != nullptr)
        setEndAngle(deg2rad(opp_atof(s)));
}

const char **cPieSliceFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_BOUNDS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_STARTANGLE, PKEY_ENDANGLE, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPieSliceFigure::moveLocal(double dx, double dy)
{
    bounds.x += dx;
    bounds.y += dy;
    fireGeometryChange();
}

void cPieSliceFigure::setBounds(const Rectangle& bounds)
{
    if (bounds == this->bounds)
        return;
    ENSURE_NONNEGATIVE(bounds.width);
    ENSURE_NONNEGATIVE(bounds.height);
    this->bounds = bounds;
    fireGeometryChange();
}

void cPieSliceFigure::setStartAngle(double startAngle)
{
    if (startAngle == this->startAngle)
        return;
    this->startAngle = startAngle;
    fireGeometryChange();
}

void cPieSliceFigure::setEndAngle(double endAngle)
{
    if (endAngle == this->endAngle)
        return;
    this->endAngle = endAngle;
    fireGeometryChange();
}

//----

void cPolygonFigure::copy(const cPolygonFigure& other)
{
    setPoints(other.getPoints());
    setSmooth(other.getSmooth());
    setJoinStyle(other.getJoinStyle());
    setFillRule(other.getFillRule());
}

void cPolygonFigure::checkIndex(int i) const
{
    if (i < 0 || i >= (int)points.size())
        throw cRuntimeError(this, "Index %d is out of range", i);
}

void cPolygonFigure::checkInsIndex(int i) const
{
    if (i < 0 || i > (int)points.size())
        throw cRuntimeError(this, "Insertion index %d is out of range", i);
}

cPolygonFigure& cPolygonFigure::operator=(const cPolygonFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPolygonFigure::info() const
{
    std::stringstream os;
    for (int i = 0; i < (int)points.size(); i++)
        os << points[i] << " ";
    return os.str();
}

void cPolygonFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    const char *s;
    setPoints(parsePoints(property, PKEY_POINTS));
    if ((s = property->getValue(PKEY_SMOOTH, 0)) != nullptr)
        setSmooth(parseBool(s));
    if ((s = property->getValue(PKEY_JOINSTYLE, 0)) != nullptr)
        setJoinStyle(parseJoinStyle(s));
    if ((s = property->getValue(PKEY_FILLRULE, 0)) != nullptr)
        setFillRule(parseFillRule(s));
}

const char **cPolygonFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POINTS, PKEY_SMOOTH, PKEY_JOINSTYLE, PKEY_FILLRULE, nullptr };
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cPolygonFigure::moveLocal(double dx, double dy)
{
    for (int i = 0; i < (int)points.size(); i++) {
        points[i].x += dx;
        points[i].y += dy;
    }
    fireGeometryChange();
}

void cPolygonFigure::setPoints(const std::vector<Point>& points)
{
    if (points == this->points)
        return;
    this->points = points;
    fireGeometryChange();
}

void cPolygonFigure::setPoint(int i, const Point& point)
{
    checkIndex(i);
    if (point == this->points[i])
        return;
    this->points[i] = point;
    fireGeometryChange();
}

void cPolygonFigure::addPoint(const Point& point)
{
    this->points.push_back(point);
    fireGeometryChange();
}

void cPolygonFigure::removePoint(int i)
{
    checkIndex(i);
    this->points.erase(this->points.begin() + i);
    fireGeometryChange();
}

void cPolygonFigure::insertPoint(int i, const Point& point)
{
    checkInsIndex(i);
    this->points.insert(this->points.begin() + i, point);
    fireGeometryChange();
}

void cPolygonFigure::setSmooth(bool smooth)
{
    if (smooth == this->smooth)
        return;
    this->smooth = smooth;
    fireVisualChange();
}

void cPolygonFigure::setJoinStyle(JoinStyle joinStyle)
{
    if (joinStyle == this->joinStyle)
        return;
    this->joinStyle = joinStyle;
    fireVisualChange();
}

void cPolygonFigure::setFillRule(FillRule fillRule)
{
    if (fillRule == this->fillRule)
        return;
    this->fillRule = fillRule;
    fireVisualChange();
}

//----

void cPathFigure::copy(const cPathFigure& other)
{
    setPath(other.getPath()); //FIXME do deep copy of structs instead!!!
    setJoinStyle(other.getJoinStyle());
    setCapStyle(other.getCapStyle());
    setOffset(other.getOffset());
    setFillRule(other.getFillRule());
}

cPathFigure& cPathFigure::operator=(const cPathFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractShapeFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cPathFigure::info() const
{
    return getPath();
}

void cPathFigure::parse(cProperty *property)
{
    cAbstractShapeFigure::parse(property);

    const char *s;
    if ((s = property->getValue(PKEY_PATH, 0)) != nullptr)
        setPath(s);
    if ((s = property->getValue(PKEY_OFFSET, 0)) != nullptr)
        setOffset(parsePoint(property, s, 0));
    if ((s = property->getValue(PKEY_JOINSTYLE, 0)) != nullptr)
        setJoinStyle(parseJoinStyle(s));
    if ((s = property->getValue(PKEY_CAPSTYLE, 0)) != nullptr)
        setCapStyle(parseCapStyle(s));
    if ((s = property->getValue(PKEY_FILLRULE, 0)) != nullptr)
        setFillRule(parseFillRule(s));
}

const char **cPathFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_PATH, PKEY_OFFSET, PKEY_JOINSTYLE, PKEY_CAPSTYLE, PKEY_FILLRULE, nullptr};
        concatArrays(keys, cAbstractShapeFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

static double getNum(const char *&s)
{
    char *end;
    double d = opp_strtod(s, &end);
    if (end == s)
        throw cRuntimeError("number expected");
    s = end;
    return d;
}

static bool getBool(const char *&s)
{
    double d = getNum(s);
    if (d != 0 && d != 1)
        throw cRuntimeError("boolean (0 or 1) expected");
    return d != 0;
}

void cPathFigure::setPath(const char *pathString)
{
    clearPath();

    const char *s = pathString;
    while (opp_isspace(*s))
        s++;
    try {
        while (*s) {
            char code = *s++;
            switch (code) {
                case 'M': {
                    double x, y;
                    x = getNum(s); y = getNum(s);
                    addMoveTo(x, y);
                    break;
                }
                case 'm': {
                    double dx, dy;
                    dx = getNum(s); dy = getNum(s);
                    addMoveRel(dx, dy);
                    break;
                }
                case 'L': {
                    double x, y;
                    x = getNum(s); y = getNum(s);
                    addLineTo(x, y);
                    break;
                }
                case 'l': {
                    double dx, dy;
                    dx = getNum(s); dy = getNum(s);
                    addLineRel(dx, dy);
                    break;
                }
                case 'H': {
                    double x = getNum(s);
                    addHorizontalLineTo(x);
                    break;
                }
                case 'h': {
                    double dx = getNum(s);
                    addHorizontalLineRel(dx);
                    break;
                }
                case 'V': {
                    double y = getNum(s);
                    addVerticalLineTo(y);
                    break;
                }
                case 'v': {
                    double dy = getNum(s);
                    addVerticalLineRel(dy);
                    break;
                }
                case 'A': {
                    double rx, ry, phi, x, y;
                    bool sweep, largeArc;
                    rx = getNum(s); ry = getNum(s); phi = getNum(s); largeArc = getBool(s); sweep = getBool(s); x = getNum(s); y = getNum(s);
                    addArcTo(rx, ry, phi, largeArc, sweep, x, y);
                    break;
                }
                case 'a': {
                    double rx, ry, phi, dx, dy;
                    bool sweep, largeArc;
                    rx = getNum(s); ry = getNum(s); phi = getNum(s); largeArc = getBool(s); sweep = getBool(s); dx = getNum(s); dy = getNum(s);
                    addArcRel(rx, ry, phi, largeArc, sweep, dx, dy);
                    break;
                }
                case 'Q': {
                    double x1, y1, x, y;
                    x1 = getNum(s); y1 = getNum(s); x = getNum(s); y = getNum(s);
                    addCurveTo(x1, y1, x, y);
                    break;
                }
                case 'q': {
                    double dx1, dy1, dx, dy;
                    dx1 = getNum(s); dy1 = getNum(s); dx = getNum(s); dy = getNum(s);
                    addCurveRel(dx1, dy1, dx, dy);
                    break;
                }
                case 'T': {
                    double x, y;
                    x = getNum(s); y = getNum(s);
                    addSmoothCurveTo(x, y);
                    break;
                }
                case 't': {
                    double dx, dy;
                    dx = getNum(s); dy = getNum(s);
                    addSmoothCurveRel(dx, dy);
                    break;
                }
                case 'C': {
                    double x1, y1, x2, y2, x, y;
                    x1 = getNum(s); y1 = getNum(s); x2 = getNum(s); y2 = getNum(s); x = getNum(s); y = getNum(s);
                    addCubicBezierCurveTo(x1, y1, x2, y2, x, y);
                    break;
                }
                case 'c': {
                    double dx1, dy1, dx2, dy2, dx, dy;
                    dx1 = getNum(s); dy1 = getNum(s); dx2 = getNum(s); dy2 = getNum(s); dx = getNum(s); dy = getNum(s);
                    addCubicBezierCurveRel(dx1, dy1, dx2, dy2, dx, dy);
                    break;
                }
                case 'S': {
                    double x2, y2, x, y;
                    x2 = getNum(s); y2 = getNum(s); x = getNum(s); y = getNum(s);
                    addSmoothCubicBezierCurveTo(x2, y2, x, y);
                    break;
                }
                case 's': {
                    double dx2, dy2, dx, dy;
                    dx2 = getNum(s); dy2 = getNum(s); dx = getNum(s); dy = getNum(s);
                    addSmoothCubicBezierCurveRel(dx2, dy2, dx, dy);
                    break;
                }
                case 'z': case 'Z': {
                    addClosePath();
                    break;
                }
                default:
                    throw cRuntimeError("unknown drawing primitive '%c'", code);
            }
            while (opp_isspace(*s))
                s++;
        }
    }
    catch (std::exception& e) {
        std::string msg = opp_stringf("%s in path near column %d", e.what(), s - pathString);
        throw cRuntimeError(msg.c_str());
    }
}

const char *cPathFigure::getPath() const
{
    // return cached copy if exists
    if (!cachedPathString.empty() || path.empty())
        return cachedPathString.c_str();

    // else produce string and cache it
    std::stringstream os;
    for (int i = 0; i < (int)path.size(); i++) {
        PathItem *base = path[i];
        os << (char)base->code << " ";
        switch (base->code) {
            case 'M': {
                MoveTo *item = static_cast<MoveTo*>(base);
                os << item->x << " " << item->y;
                break;
            }
            case 'm': {
                MoveRel *item = static_cast<MoveRel*>(base);
                os << item->dx << " " << item->dy;
                break;
            }
            case 'L': {
                LineTo *item = static_cast<LineTo*>(base);
                os << item->x << " " << item->y;
                break;
            }
            case 'l': {
                LineRel *item = static_cast<LineRel*>(base);
                os << item->dx << " " << item->dy;
                break;
            }
            case 'H': {
                HorizLineTo *item = static_cast<HorizLineTo*>(base);
                os << item->x;
                break;
            }
            case 'h': {
                HorizLineRel *item = static_cast<HorizLineRel*>(base);
                os << item->dx;
                break;
            }
            case 'V': {
                VertLineTo *item = static_cast<VertLineTo*>(base);
                os << item->y;
                break;
            }
            case 'v': {
                VertLineRel *item = static_cast<VertLineRel*>(base);
                os << item->dy;
                break;
            }
            case 'A': {
                ArcTo *item = static_cast<ArcTo*>(base);
                os << item->rx << " " << item->ry << " " << item->phi << " " << item->largeArc << " " << item->sweep << " " << item->x << " " << item->y;
                break;
            }
            case 'a': {
                ArcRel *item = static_cast<ArcRel*>(base);
                os << item->rx << " " << item->ry << " " << item->phi << " " << item->largeArc << " " << item->sweep << " " << item->dx << " " << item->dy;
                break;
            }
            case 'Q': {
                CurveTo *item = static_cast<CurveTo*>(base);
                os << item->x1 << " " << item->y1 << " " << item->x << " " << item->y;
                break;
            }
            case 'q': {
                CurveRel *item = static_cast<CurveRel*>(base);
                os << item->dx1 << " " << item->dy1 << " " << item->dx << " " << item->dy;
                break;
            }
            case 'T': {
                SmoothCurveTo *item = static_cast<SmoothCurveTo*>(base);
                os << item->x << " " << item->y;
                break;
            }
            case 't': {
                SmoothCurveRel *item = static_cast<SmoothCurveRel*>(base);
                os << item->dx << " " << item->dy;
                break;
            }
            case 'C': {
                CubicBezierCurveTo *item = static_cast<CubicBezierCurveTo*>(base);
                os << item->x1 << " " << item->y1 << " " << item->x2 << " " << item->y2 << " " << item->x << " " << item->y;
                break;
            }
            case 'c': {
                CubicBezierCurveRel *item = static_cast<CubicBezierCurveRel*>(base);
                os << item->dx1 << " " << item->dy1 << " " << item->dx2 << " " << item->dy2 << " " << item->dx << " " << item->dy;
                break;
            }
            case 'S': {
                SmoothCubicBezierCurveTo *item = static_cast<SmoothCubicBezierCurveTo*>(base);
                os << item->x2 << " " << item->y2 << " " << item->x << " " << item->y;
                break;
            }
            case 's': {
                SmoothCubicBezierCurveRel *item = static_cast<SmoothCubicBezierCurveRel*>(base);
                os << item->dx2 << " " << item->dy2 << " " << item->dx << " " << item->dy;
                break;
            }
            case 'Z': {
                break;
            }
            default:
                throw cRuntimeError(this, "unknown path item '%c'", base->code);
        }
        os << " ";
    }
    cachedPathString = os.str();
    return cachedPathString.c_str();
}

void cPathFigure::addItem(PathItem *item)
{
    path.push_back(item);
    if (!cachedPathString.empty())
        cachedPathString.clear();
    fireGeometryChange();
}

void cPathFigure::doClearPath()
{
    for (int i = 0; i < (int)path.size(); i++)
        delete path[i];
    path.clear();
    cachedPathString.clear();
}

void cPathFigure::clearPath()
{
    doClearPath();
    fireGeometryChange();
}

void cPathFigure::addMoveTo(double x, double y)
{
    MoveTo *item = new MoveTo();
    item->code = 'M';
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addMoveRel(double dx, double dy)
{
    MoveRel *item = new MoveRel();
    item->code = 'm';
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addLineTo(double x, double y)
{
    LineTo *item = new LineTo();
    item->code = 'L';
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addLineRel(double dx, double dy)
{
    LineRel *item = new LineRel();
    item->code = 'l';
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addHorizontalLineTo(double x)
{
    HorizLineTo *item = new HorizLineTo();
    item->code = 'H';
    item->x = x;
    addItem(item);
}

void cPathFigure::addHorizontalLineRel(double dx)
{
    HorizLineRel *item = new HorizLineRel();
    item->code = 'h';
    item->dx = dx;
    addItem(item);
}

void cPathFigure::addVerticalLineTo(double y)
{
    VertLineTo *item = new VertLineTo();
    item->code = 'V';
    item->y = y;
    addItem(item);
}

void cPathFigure::addVerticalLineRel(double dy)
{
    VertLineRel *item = new VertLineRel();
    item->code = 'v';
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addArcTo(double rx, double ry, double phi, bool largeArc, bool sweep, double x, double y)
{
    ArcTo *item = new ArcTo();
    item->code = 'A';
    item->rx = rx;
    item->ry = ry;
    item->phi = phi;
    item->largeArc = largeArc;
    item->sweep = sweep;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addArcRel(double rx, double ry, double phi, bool largeArc, bool sweep, double dx, double dy)
{
    ArcRel *item = new ArcRel();
    item->code = 'a';
    item->rx = rx;
    item->ry = ry;
    item->phi = phi;
    item->largeArc = largeArc;
    item->sweep = sweep;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addCurveTo(double x1, double y1, double x, double y)
{
    CurveTo *item = new CurveTo();
    item->code = 'Q';
    item->x1 = x1;
    item->y1 = y1;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addCurveRel(double dx1, double dy1, double dx, double dy)
{
    CurveRel *item = new CurveRel();
    item->code = 'q';
    item->dx1 = dx1;
    item->dy1 = dy1;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addSmoothCurveTo(double x, double y)
{
    SmoothCurveTo *item = new SmoothCurveTo();
    item->code = 'T';
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addSmoothCurveRel(double dx, double dy)
{
    SmoothCurveRel *item = new SmoothCurveRel();
    item->code = 't';
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addCubicBezierCurveTo(double x1, double y1, double x2, double y2, double x, double y)
{
    CubicBezierCurveTo *item = new CubicBezierCurveTo();
    item->code = 'C';
    item->x1 = x1;
    item->y1 = y1;
    item->x2 = x2;
    item->y2 = y2;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addCubicBezierCurveRel(double dx1, double dy1, double dx2, double dy2, double dx, double dy)
{
    CubicBezierCurveRel *item = new CubicBezierCurveRel();
    item->code = 'c';
    item->dx1 = dx1;
    item->dy1 = dy1;
    item->dx2 = dx2;
    item->dy2 = dy2;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addSmoothCubicBezierCurveTo(double x2, double y2, double x, double y)
{
    SmoothCubicBezierCurveTo *item = new SmoothCubicBezierCurveTo();
    item->code = 'S';
    item->x2 = x2;
    item->y2 = y2;
    item->x = x;
    item->y = y;
    addItem(item);
}

void cPathFigure::addSmoothCubicBezierCurveRel(double dx2, double dy2, double dx, double dy)
{
    SmoothCubicBezierCurveRel *item = new SmoothCubicBezierCurveRel();
    item->code = 's';
    item->dx2 = dx2;
    item->dy2 = dy2;
    item->dx = dx;
    item->dy = dy;
    addItem(item);
}

void cPathFigure::addClosePath()
{
    ClosePath *item = new ClosePath();
    item->code = 'Z';
    addItem(item);
}

void cPathFigure::moveLocal(double dx, double dy)
{
    setOffset(Point(getOffset()).translate(dx, dy));
}

void cPathFigure::setJoinStyle(JoinStyle joinStyle)
{
    if (joinStyle == this->joinStyle)
        return;
    this->joinStyle = joinStyle;
    fireVisualChange();
}

void cPathFigure::setCapStyle(CapStyle capStyle)
{
    if (capStyle == this->capStyle)
        return;
    this->capStyle = capStyle;
    fireVisualChange();
}

void cPathFigure::setFillRule(FillRule fillRule)
{
    if (fillRule == this->fillRule)
        return;
    this->fillRule = fillRule;
    fireVisualChange();
}

//----

void cAbstractTextFigure::copy(const cAbstractTextFigure& other)
{
    setPosition(other.getPosition());
    setColor(other.getColor());
    setOpacity(other.getOpacity());
    setFont(other.getFont());
    setText(other.getText());
    setAnchor(other.getAnchor());
}

cAbstractTextFigure& cAbstractTextFigure::operator=(const cAbstractTextFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

std::string cAbstractTextFigure::info() const
{
    std::stringstream os;
    os << "\"" << getText() << "\" at " << getPosition();
    return os.str();
}

void cAbstractTextFigure::parse(cProperty *property)
{
    cFigure::parse(property);

    const char *s;
    setPosition(parsePoint(property, PKEY_POS, 0));
    setText(opp_nulltoempty(property->getValue(PKEY_TEXT)));
    if ((s = property->getValue(PKEY_COLOR)) != nullptr)
        setColor(parseColor(s));
    if ((s = property->getValue(PKEY_OPACITY)) != nullptr)
        setOpacity(opp_atof(s));
    if (property->containsKey(PKEY_FONT))
        setFont(parseFont(property, PKEY_FONT));
    if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
        setAnchor(parseAnchor(s));
}

const char **cAbstractTextFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_POS, PKEY_TEXT, PKEY_COLOR, PKEY_OPACITY, PKEY_FONT, PKEY_ANCHOR, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractTextFigure::moveLocal(double dx, double dy)
{
    position.x += dx;
    position.y += dy;
    fireGeometryChange();
}

void cAbstractTextFigure::setPosition(const Point& position)
{
    if (position == this->position)
        return;
    this->position = position;
    fireGeometryChange();
}

void cAbstractTextFigure::setAnchor(Anchor anchor)
{
    if (anchor == this->anchor)
        return;
    this->anchor = anchor;
    fireGeometryChange();
}

void cAbstractTextFigure::setColor(const Color& color)
{
    if (color == this->color)
        return;
    this->color = color;
    fireVisualChange();
}

void cAbstractTextFigure::setOpacity(double opacity)
{
    if (opacity == this->opacity)
        return;
    ENSURE_RANGE01(opacity);
    this->opacity = opacity;
    fireVisualChange();
}

void cAbstractTextFigure::setFont(Font font)
{
    if (font == this->font)
        return;
    this->font = font;
    fireVisualChange();
}

void cAbstractTextFigure::setText(const char* text)
{
    if (text == this->text)
        return;
    this->text = text;
    fireInputDataChange();
}

cFigure::Rectangle cAbstractTextFigure::getBounds() const
{
    int width, height, ascent;
    getEnvir()->getTextExtent(getFont(), getText(), width, height, ascent);
    return computeBoundingBox(position, Point(width, height), ascent, anchor);
}

//----

cTextFigure& cTextFigure::operator=(const cTextFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractTextFigure::operator=(other);
    copy(other);
    return *this;
}

//----

cLabelFigure& cLabelFigure::operator=(const cLabelFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractTextFigure::operator=(other);
    copy(other);
    return *this;
}

//----

void cAbstractImageFigure::copy(const cAbstractImageFigure& other)
{
    setPosition(other.getPosition());
    setAnchor(other.getAnchor());
    setWidth(other.getWidth());
    setHeight(other.getHeight());
    setInterpolation(other.getInterpolation());
    setOpacity(other.getOpacity());
    setTintColor(other.getTintColor());
    setTintAmount(other.getTintAmount());
}

cAbstractImageFigure& cAbstractImageFigure::operator=(const cAbstractImageFigure& other)
{
    if (this == &other)
        return *this;
    cFigure::operator=(other);
    copy(other);
    return *this;
}

void cAbstractImageFigure::parse(cProperty *property)
{
    cFigure::parse(property);

    const char *s;
    if (property->containsKey(PKEY_BOUNDS)) {
        if (property->containsKey(PKEY_POS) || property->containsKey(PKEY_SIZE) || property->containsKey(PKEY_ANCHOR))
            throw cRuntimeError("%s, %s and %s are not allowed when %s is present", PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_BOUNDS);
        if (property->getNumValues(PKEY_BOUNDS) != 4)
            throw cRuntimeError("%s: x, y, width, height expected", PKEY_BOUNDS);
        Point p = parsePoint(property, PKEY_BOUNDS, 0);
        Point size = parsePoint(property, PKEY_BOUNDS, 2);
        setPosition(p);
        setAnchor(ANCHOR_NW);
        setWidth(size.x);
        setHeight(size.y);
    }
    else {
        setPosition(parsePoint(property, PKEY_POS, 0));
        if ((s = property->getValue(PKEY_ANCHOR)) != nullptr)
            setAnchor(parseAnchor(s));
        Point size = parsePoint(property, PKEY_SIZE, 0);
        setWidth(size.x);
        setHeight(size.y);
    }
    if ((s = property->getValue(PKEY_INTERPOLATION)) != nullptr)
        setInterpolation(parseInterpolation(s));
    if ((s = property->getValue(PKEY_OPACITY)) != nullptr)
        setOpacity(opp_atof(s));
    if ((s = property->getValue(PKEY_TINT, 0)) != nullptr) {
        setTintColor(parseColor(s));
        setTintAmount(0.5);
    }
    if ((s = property->getValue(PKEY_TINT, 1)) != nullptr)
        setTintAmount(opp_atof(s));
}

const char **cAbstractImageFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_BOUNDS, PKEY_POS, PKEY_SIZE, PKEY_ANCHOR, PKEY_INTERPOLATION, PKEY_OPACITY, PKEY_TINT, nullptr};
        concatArrays(keys, cFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cAbstractImageFigure::moveLocal(double dx, double dy)
{
    position.x += dx;
    position.y += dy;
    fireGeometryChange();
}

void cAbstractImageFigure::setPosition(const Point& position)
{
    if (position == this->position)
        return;
    this->position = position;
    fireGeometryChange();
}

void cAbstractImageFigure::setAnchor(Anchor anchor)
{
    if (anchor == this->anchor)
        return;
    this->anchor = anchor;
    fireGeometryChange();
}

void cAbstractImageFigure::setWidth(double width)
{
    if (width == this->width)
        return;
    ENSURE_NONNEGATIVE(width);
    this->width = width;
    fireGeometryChange();
}

void cAbstractImageFigure::setHeight(double height)
{
    if (height == this->height)
        return;
    ENSURE_NONNEGATIVE(height);
    this->height = height;
    fireGeometryChange();
}

void cAbstractImageFigure::setInterpolation(Interpolation interpolation)
{
    if (interpolation == this->interpolation)
        return;
    this->interpolation = interpolation;
    fireVisualChange();
}

void cAbstractImageFigure::setOpacity(double opacity)
{
    if (opacity == this->opacity)
        return;
    ENSURE_RANGE01(opacity);
    this->opacity = opacity;
    fireVisualChange();
}

void cAbstractImageFigure::setTintColor(const Color& tintColor)
{
    if (tintColor == this->tintColor)
        return;
    this->tintColor = tintColor;
    fireVisualChange();
}

void cAbstractImageFigure::setTintAmount(double tintAmount)
{
    if (tintAmount == this->tintAmount)
        return;
    ENSURE_RANGE01(tintAmount);
    this->tintAmount = tintAmount;
    fireVisualChange();
}

cFigure::Rectangle cAbstractImageFigure::getBounds() const
{
    Point size = Point(width, height);
    if (size.x == 0 || size.y == 0) {
        Point defaultSize = getDefaultSize();
        if (size.x == 0)
            size.x = defaultSize.x;
        if (size.y == 0)
            size.y = defaultSize.y;
    }
    return computeBoundingBox(position, size, size.y, anchor);
}

//----

std::string cImageFigure::info() const
{
    std::stringstream os;
    os << "\"" << getImageName() << "\" at " << getPosition();
    return os.str();
}

cImageFigure& cImageFigure::operator=(const cImageFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractImageFigure::operator=(other);
    copy(other);
    return *this;
}

void cImageFigure::copy(const cImageFigure& other)
{
    setImageName(other.getImageName());
}

void cImageFigure::parse(cProperty *property)
{
    setImageName(opp_nulltoempty(property->getValue(PKEY_IMAGE)));
    cAbstractImageFigure::parse(property);
}

const char **cImageFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_IMAGE, nullptr};
        concatArrays(keys, cAbstractImageFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

void cImageFigure::setImageName(const char* imageName)
{
    if (imageName == this->imageName)
        return;
    this->imageName = imageName;
    fireInputDataChange();
}

cFigure::Point cImageFigure::getDefaultSize() const
{
    int width, height;
    getEnvir()->getImageSize(getImageName(), width, height);
    return Point(width, height);
}

//----

cIconFigure& cIconFigure::operator=(const cIconFigure& other)
{
    if (this == &other) return *this;
    cImageFigure::operator=(other);
    copy(other);
    return *this;
}

//----

std::string cPixmapFigure::info() const
{
    std::stringstream os;
    os << "(" << getPixmapWidth() << " x " << getPixmapHeight() << ") at " << getPosition();
    return os.str();
}

cPixmapFigure& cPixmapFigure::operator=(const cPixmapFigure& other)
{
    if (this == &other)
        return *this;
    cAbstractImageFigure::operator=(other);
    copy(other);
    return *this;
}

void cPixmapFigure::copy(const cPixmapFigure& other)
{
    setPixmap(other.getPixmap());
}

void cPixmapFigure::parse(cProperty *property)
{
    cAbstractImageFigure::parse(property);

    int width = 0, height = 0;
    const char *s;
    if ((s = property->getValue(PKEY_RESOLUTION, 0)) != nullptr)
        width = opp_atoul(s);
    if ((s = property->getValue(PKEY_RESOLUTION, 1)) != nullptr)
        height = opp_atoul(s);
    if (width > 0 || height > 0)
        setPixmap(Pixmap(width, height));
}

const char **cPixmapFigure::getAllowedPropertyKeys() const
{
    static const char *keys[32];
    if (!keys[0]) {
        const char *localKeys[] = { PKEY_RESOLUTION, nullptr };
        concatArrays(keys, cAbstractImageFigure::getAllowedPropertyKeys(), localKeys);
    }
    return keys;
}

cFigure::Point cPixmapFigure::getDefaultSize() const
{
    return Point(pixmap.getWidth(), pixmap.getHeight());
}

void cPixmapFigure::setPixmap(const Pixmap& pixmap)
{
    if (&pixmap != &this->pixmap) // don't copy if it's the same object
        this->pixmap = pixmap;
    fireInputDataChange(); // always notify, as content may have changed
}

void cPixmapFigure::setPixmapSize(int width, int height, const RGBA& fill)
{
    pixmap.setSize(width, height, fill);
    fireInputDataChange();
}

void cPixmapFigure::setPixmapSize(int width, int height, const Color& color, double opacity)
{
    if (pixmap.getWidth() == width && pixmap.getHeight() == height)
        return;
    ENSURE_RANGE01(opacity);
    pixmap.setSize(width, height, color, opacity);
    fireInputDataChange();
}

void cPixmapFigure::fillPixmap(const RGBA& fill)
{
    pixmap.fill(fill);
    fireInputDataChange();
}

void cPixmapFigure::fillPixmap(const Color& color, double opacity)
{
    ENSURE_RANGE01(opacity);
    pixmap.fill(color, opacity);
    fireInputDataChange();
}

void cPixmapFigure::setPixel(int x, int y, const RGBA& argb)
{
    pixmap.pixel(x, y) = argb;
    fireInputDataChange();
}

void cPixmapFigure::setPixel(int x, int y, const Color& color, double opacity)
{
    ENSURE_RANGE01(opacity);
    pixmap.setPixel(x, y, color, opacity);
    fireInputDataChange();
}

void cPixmapFigure::setPixelColor(int x, int y, const Color& color)
{
    pixmap.setColor(x, y, color);
    fireInputDataChange();
}

void cPixmapFigure::setPixelOpacity(int x, int y, double opacity)
{
    ENSURE_RANGE01(opacity);
    pixmap.setOpacity(x, y, opacity);
    fireInputDataChange();
}

//------

cCanvas::cCanvas(const char *name) : cOwnedObject(name), backgroundColor(cFigure::Color(160,224,160))
{
    rootFigure = new cGroupFigure("rootFigure");
    take(rootFigure);
}

cCanvas::~cCanvas()
{
    dropAndDelete(rootFigure);
}

void cCanvas::copy(const cCanvas& other)
{
    setBackgroundColor(other.getBackgroundColor());
    dropAndDelete(rootFigure);
    tagBitIndex.clear();
    rootFigure = other.getRootFigure()->dupTree();
    take(rootFigure);
}

cCanvas& cCanvas::operator=(const cCanvas& other)
{
    if (this == &other)
        return *this;
    cOwnedObject::operator=(other);
    copy(other);
    return *this;
}

void cCanvas::forEachChild(cVisitor *v)
{
    rootFigure->forEachChild(v);  // skip the root figure from the tree
}

std::string cCanvas::info() const
{
    std::stringstream os;
    os << rootFigure->getNumFigures() << " toplevel figure(s)";
    return os.str();
}

bool cCanvas::containsCanvasItems(cProperties *properties)
{
    for (int i = 0; i < properties->getNumProperties(); i++) {
        cProperty *property = properties->get(i);
        if (property->isName("figure"))
            return true;
    }
    return false;
}

void cCanvas::addFiguresFrom(cProperties *properties)
{
    std::map<cFigure *, double> orderMap;

    // Note: the following code assumes that parent figures precede their children, otherwise a "parent not found" error will occur
    for (int i = 0; i < properties->getNumProperties(); i++) {
        cProperty *property = properties->get(i);
        if (property->isName("figure"))
            parseFigure(property, orderMap);
    }
}

cFigure *cCanvas::parseFigure(cProperty *property, std::map<cFigure *, double>& orderMap) const
{
    try {
        const char *path = property->getIndex();
        if (!path)
            throw cRuntimeError("@figure property is expected to have an index which will become the figure name, e.g. @figure[foo]");
        const char *lastDot = strrchr(path, '.');
        cFigure *parent;
        const char *name;
        if (lastDot) {
            std::string parentPath = std::string(path, lastDot - path);
            parent = getFigureByPath(parentPath.c_str());
            if (!parent)
                throw cRuntimeError("parent figure \"%s\" not found", parentPath.c_str());
            name = lastDot + 1;
        }
        else {
            parent = rootFigure;
            name = path;
        }
        if (!name[0])
            throw cRuntimeError("figure name cannot be empty");

        cFigure *figure = parent->getFigure(name);
        if (!figure) {
            const char *type = opp_nulltoempty(property->getValue(PKEY_TYPE));
            figure = createFigure(type);
            figure->setName(name);
            const char *order = property->getValue(PKEY_CHILDZ);
            if (order)
                orderMap[figure] = opp_atof(order);
            parent->insertChild(figure, orderMap);
        }
        else {
            figure->raiseToTop();
        }

        figure->parse(property);
        return figure;
    }
    catch (std::exception& e) {
        throw cRuntimeError(this, "Error creating figure from NED property @%s: %s", property->getFullName(), e.what());
    }
}

cFigure *cCanvas::createFigure(const char *type) const
{
    cFigure *figure;
    if (!strcmp(type, "group"))
        figure = new cGroupFigure();
//    else if (!strcmp(type, "panel"))
//        figure = new cPanelFigure();
    else if (!strcmp(type, "line"))
        figure = new cLineFigure();
    else if (!strcmp(type, "arc"))
        figure = new cArcFigure();
    else if (!strcmp(type, "polyline"))
        figure = new cPolylineFigure();
    else if (!strcmp(type, "rectangle"))
        figure = new cRectangleFigure();
    else if (!strcmp(type, "oval"))
        figure = new cOvalFigure();
    else if (!strcmp(type, "ring"))
        figure = new cRingFigure();
    else if (!strcmp(type, "pieslice"))
        figure = new cPieSliceFigure();
    else if (!strcmp(type, "polygon"))
        figure = new cPolygonFigure();
    else if (!strcmp(type, "path"))
        figure = new cPathFigure();
    else if (!strcmp(type, "text"))
        figure = new cTextFigure();
    else if (!strcmp(type, "label"))
        figure = new cLabelFigure();
    else if (!strcmp(type, "image"))
        figure = new cImageFigure();
    else if (!strcmp(type, "icon"))
        figure = new cIconFigure();
    else if (!strcmp(type, "pixmap"))
        figure = new cPixmapFigure();
    else {
        // find implementor class
        cObjectFactory *factory = cObjectFactory::find(type);  // try as is
        std::string className;
        if (!factory) {
            // try capitalizing and adding "Figure" (mind possibly present namespace!)
            className = std::string(type) + "Figure";
            int pos = className.rfind("::");
            int nameStart = pos==std::string::npos ? 0 : pos+2;
            className[nameStart] = opp_toupper(className[nameStart]);
            factory = cObjectFactory::find(className.c_str());
        }
        if (!factory)
            throw cRuntimeError("Implementation class for figure not found (tried '%s' and '%s')", type, className.c_str());
        cObject *obj = factory->createOne();
        figure = dynamic_cast<cFigure *>(obj);
        if (!figure)
            throw cRuntimeError("Wrong figure class: cannot cast %s to cFigure", obj->getClassName());
    }
    return figure;
}

void cCanvas::dumpSupportedPropertyKeys(std::ostream& out) const
{
    const char *types[] = {
        "group", /*"panel",*/ "line", "arc", "polyline", "rectangle", "oval", "ring", "pieslice",
        "polygon", "path", "text", "label", "image", "pixmap", nullptr
    };

    for (const char **p = types; *p; p++) {
        const char *type = *p;
        cFigure *figure = createFigure(type);
        out << type << ": " << opp_join(figure->getAllowedPropertyKeys(), ", ") << "\n";
        delete figure;
    }
}

cFigure *cCanvas::getSubmodulesLayer() const
{
    return rootFigure->getFigure("submodules");
}

uint64_t cCanvas::parseTags(const char *s)
{
    uint64_t result = 0;
    cStringTokenizer tokenizer(s);
    while (tokenizer.hasMoreTokens()) {
        const char *tag = tokenizer.nextToken();
        int bitIndex;
        std::map<std::string, int>::const_iterator it = tagBitIndex.find(tag);
        if (it != tagBitIndex.end())
            bitIndex = it->second;
        else {
            bitIndex = tagBitIndex.size();
            if (bitIndex >= 64)
                throw cRuntimeError(this, "Cannot add figure tag \"%s\": maximum 64 tags supported", tag);
            tagBitIndex[tag] = bitIndex;
        }
        result |= ((uint64_t)1) << bitIndex;
    }
    return result;
}

std::string cCanvas::getTags(uint64_t tagBits)
{
    std::stringstream os;
    for (std::map<std::string, int>::const_iterator it = tagBitIndex.begin(); it != tagBitIndex.end(); ++it) {
        if ((tagBits & (((uint64_t)1) << it->second)) != 0) {
            if (it != tagBitIndex.begin())
                os << " ";
            os << it->first;
        }
    }
    return os.str();
}

std::string cCanvas::getAllTags() const
{
    std::stringstream os;
    for (std::map<std::string, int>::const_iterator it = tagBitIndex.begin(); it != tagBitIndex.end(); ++it) {
        if (it != tagBitIndex.begin())
            os << " ";
        os << it->first;
    }
    return os.str();
}

std::vector<std::string> cCanvas::getAllTagsAsVector() const
{
    std::vector<std::string> result;
    for (std::map<std::string, int>::const_iterator it = tagBitIndex.begin(); it != tagBitIndex.end(); ++it)
        result.push_back(it->first);
    return result;
}

}  // namespace omnetpp

