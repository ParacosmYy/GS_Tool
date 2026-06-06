#include "i28328/m28328.h"
QVector<double> m28328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
