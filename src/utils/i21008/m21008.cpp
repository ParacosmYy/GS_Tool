#include "i21008/m21008.h"
QVector<double> m21008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
