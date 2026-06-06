#include "b8761/m8761.h"
QVector<double> m8761::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
