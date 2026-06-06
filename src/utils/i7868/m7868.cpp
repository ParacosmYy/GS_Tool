#include "i7868/m7868.h"
QVector<double> m7868::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
