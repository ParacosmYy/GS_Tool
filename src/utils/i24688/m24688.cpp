#include "i24688/m24688.h"
QVector<double> m24688::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
