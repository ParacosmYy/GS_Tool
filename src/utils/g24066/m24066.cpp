#include "g24066/m24066.h"
QVector<double> m24066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
