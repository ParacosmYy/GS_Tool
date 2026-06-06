#include "l9051/m9051.h"
QVector<double> m9051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
