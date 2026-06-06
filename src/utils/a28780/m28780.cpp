#include "a28780/m28780.h"
QVector<double> m28780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
