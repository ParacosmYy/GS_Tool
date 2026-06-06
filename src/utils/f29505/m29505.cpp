#include "f29505/m29505.h"
QVector<double> m29505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
