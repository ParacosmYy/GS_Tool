#include "o8914/m8914.h"
QVector<double> m8914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
