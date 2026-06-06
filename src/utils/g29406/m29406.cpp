#include "g29406/m29406.h"
QVector<double> m29406::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
