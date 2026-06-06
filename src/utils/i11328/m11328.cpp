#include "i11328/m11328.h"
QVector<double> m11328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
