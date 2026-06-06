#include "o16914/m16914.h"
QVector<double> m16914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
