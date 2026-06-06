#include "i16008/m16008.h"
QVector<double> m16008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
