#include "i7968/m7968.h"
QVector<double> m7968::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
