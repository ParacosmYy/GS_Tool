#include "i16828/m16828.h"
QVector<double> m16828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
