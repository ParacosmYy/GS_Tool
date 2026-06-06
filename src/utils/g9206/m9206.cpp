#include "g9206/m9206.h"
QVector<double> m9206::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
