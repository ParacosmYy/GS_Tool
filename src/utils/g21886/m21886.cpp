#include "g21886/m21886.h"
QVector<double> m21886::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
