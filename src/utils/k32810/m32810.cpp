#include "k32810/m32810.h"
QVector<double> m32810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
