#include "m29492/m29492.h"
QVector<double> m29492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
