#include "h25067/m25067.h"
QVector<double> m25067::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
