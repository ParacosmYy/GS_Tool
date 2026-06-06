#include "l8551/m8551.h"
QVector<double> m8551::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
