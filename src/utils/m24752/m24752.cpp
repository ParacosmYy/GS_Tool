#include "m24752/m24752.h"
QVector<double> m24752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
