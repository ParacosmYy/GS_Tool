#include "p25775/m25775.h"
QVector<double> m25775::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
