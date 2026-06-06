#include "k21250/m21250.h"
QVector<double> m21250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
