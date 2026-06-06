#include "k15850/m15850.h"
QVector<double> m15850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
