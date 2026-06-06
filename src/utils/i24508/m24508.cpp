#include "i24508/m24508.h"
QVector<double> m24508::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
