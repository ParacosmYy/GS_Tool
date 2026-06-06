#include "g21306/m21306.h"
QVector<double> m21306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
