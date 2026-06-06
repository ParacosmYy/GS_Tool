#include "m9932/m9932.h"
QVector<double> m9932::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
