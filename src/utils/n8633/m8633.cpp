#include "n8633/m8633.h"
QVector<double> m8633::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
