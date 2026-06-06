#include "l21911/m21911.h"
QVector<double> m21911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
