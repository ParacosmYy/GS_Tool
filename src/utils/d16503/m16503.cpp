#include "d16503/m16503.h"
QVector<double> m16503::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
