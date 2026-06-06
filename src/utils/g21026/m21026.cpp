#include "g21026/m21026.h"
QVector<double> m21026::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
