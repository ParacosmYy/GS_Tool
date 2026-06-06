#include "d25063/m25063.h"
QVector<double> m25063::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
