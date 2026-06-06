#include "l24111/m24111.h"
QVector<double> m24111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
