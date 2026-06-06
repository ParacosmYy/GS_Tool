#include "m24472/m24472.h"
QVector<double> m24472::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
