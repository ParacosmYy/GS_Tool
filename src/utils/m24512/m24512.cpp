#include "m24512/m24512.h"
QVector<double> m24512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
