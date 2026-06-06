#include "i28988/m28988.h"
QVector<double> m28988::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
