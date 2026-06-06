#include "i24068/m24068.h"
QVector<double> m24068::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
