#include "k21050/m21050.h"
QVector<double> m21050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
