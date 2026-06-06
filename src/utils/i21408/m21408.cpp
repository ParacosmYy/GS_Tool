#include "i21408/m21408.h"
QVector<double> m21408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
