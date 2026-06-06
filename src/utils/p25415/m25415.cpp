#include "p25415/m25415.h"
QVector<double> m25415::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
