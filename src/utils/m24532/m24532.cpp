#include "m24532/m24532.h"
QVector<double> m24532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
