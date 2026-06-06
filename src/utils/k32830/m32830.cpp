#include "k32830/m32830.h"
QVector<double> m32830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
