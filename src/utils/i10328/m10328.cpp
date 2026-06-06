#include "i10328/m10328.h"
QVector<double> m10328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
