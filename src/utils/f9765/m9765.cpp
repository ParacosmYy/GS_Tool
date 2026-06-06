#include "f9765/m9765.h"
QVector<double> m9765::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
