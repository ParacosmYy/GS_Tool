#include "k17850/m17850.h"
QVector<double> m17850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
