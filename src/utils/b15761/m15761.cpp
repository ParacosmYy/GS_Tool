#include "b15761/m15761.h"
QVector<double> m15761::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
