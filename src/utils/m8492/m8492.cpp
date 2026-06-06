#include "m8492/m8492.h"
QVector<double> m8492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
