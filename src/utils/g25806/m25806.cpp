#include "g25806/m25806.h"
QVector<double> m25806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
