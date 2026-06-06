#include "g21646/m21646.h"
QVector<double> m21646::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
