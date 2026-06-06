#include "p25055/m25055.h"
QVector<double> m25055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
