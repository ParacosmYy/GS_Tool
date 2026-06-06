#include "p24015/m24015.h"
QVector<double> m24015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
