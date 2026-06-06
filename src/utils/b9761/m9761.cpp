#include "b9761/m9761.h"
QVector<double> m9761::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
