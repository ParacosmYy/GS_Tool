#include "t8259/m8259.h"
QVector<double> m8259::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
