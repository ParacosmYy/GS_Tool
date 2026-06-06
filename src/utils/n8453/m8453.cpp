#include "n8453/m8453.h"
QVector<double> m8453::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
