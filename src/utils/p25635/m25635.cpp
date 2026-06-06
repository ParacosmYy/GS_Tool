#include "p25635/m25635.h"
QVector<double> m25635::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
