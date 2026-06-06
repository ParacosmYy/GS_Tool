#include "i13008/m13008.h"
QVector<double> m13008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
