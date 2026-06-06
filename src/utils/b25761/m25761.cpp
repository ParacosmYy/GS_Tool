#include "b25761/m25761.h"
QVector<double> m25761::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
