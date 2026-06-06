#include "o8014/m8014.h"
QVector<double> m8014::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
