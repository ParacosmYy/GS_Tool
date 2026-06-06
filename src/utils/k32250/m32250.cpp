#include "k32250/m32250.h"
QVector<double> m32250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
