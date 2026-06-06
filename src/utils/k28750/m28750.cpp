#include "k28750/m28750.h"
QVector<double> m28750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
