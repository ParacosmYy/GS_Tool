#include "g21806/m21806.h"
QVector<double> m21806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
