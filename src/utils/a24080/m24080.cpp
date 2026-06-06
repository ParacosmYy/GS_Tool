#include "a24080/m24080.h"
QVector<double> m24080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
