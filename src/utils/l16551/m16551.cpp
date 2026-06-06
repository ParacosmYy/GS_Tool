#include "l16551/m16551.h"
QVector<double> m16551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
