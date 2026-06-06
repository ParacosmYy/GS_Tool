#include "i9008/m9008.h"
QVector<double> m9008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
