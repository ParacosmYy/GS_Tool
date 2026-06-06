#include "g29886/m29886.h"
QVector<double> m29886::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
