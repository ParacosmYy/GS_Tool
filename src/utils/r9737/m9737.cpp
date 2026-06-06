#include "r9737/m9737.h"
QVector<double> m9737::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
