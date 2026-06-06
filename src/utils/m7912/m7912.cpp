#include "m7912/m7912.h"
QVector<double> m7912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
