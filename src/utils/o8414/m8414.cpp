#include "o8414/m8414.h"
QVector<double> m8414::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
