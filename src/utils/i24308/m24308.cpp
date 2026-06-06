#include "i24308/m24308.h"
QVector<double> m24308::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
