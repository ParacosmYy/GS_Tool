#include "n8113/m8113.h"
QVector<double> m8113::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
