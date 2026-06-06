#include "m21232/m21232.h"
QVector<double> m21232::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
