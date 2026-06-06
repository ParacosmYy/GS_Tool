#include "q29256/m29256.h"
QVector<double> m29256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
