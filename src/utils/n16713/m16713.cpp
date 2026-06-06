#include "n16713/m16713.h"
QVector<double> m16713::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
