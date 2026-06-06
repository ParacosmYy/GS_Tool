#include "p25615/m25615.h"
QVector<double> m25615::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
