#include "p18055/m18055.h"
QVector<double> m18055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
