#include "e25024/m25024.h"
QVector<double> m25024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
