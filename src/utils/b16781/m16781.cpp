#include "b16781/m16781.h"
QVector<double> m16781::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
