#include "g25066/m25066.h"
QVector<double> m25066::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
