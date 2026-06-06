#include "d25503/m25503.h"
QVector<double> m25503::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
