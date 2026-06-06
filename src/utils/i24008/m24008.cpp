#include "i24008/m24008.h"
QVector<double> m24008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
