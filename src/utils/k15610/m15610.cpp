#include "k15610/m15610.h"
QVector<double> m15610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
