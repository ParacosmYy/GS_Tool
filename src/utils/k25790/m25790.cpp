#include "k25790/m25790.h"
QVector<double> m25790::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
