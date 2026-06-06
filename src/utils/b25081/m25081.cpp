#include "b25081/m25081.h"
QVector<double> m25081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
