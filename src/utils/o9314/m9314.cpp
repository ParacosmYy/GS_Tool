#include "o9314/m9314.h"
QVector<double> m9314::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
