#include "b18761/m18761.h"
QVector<double> m18761::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
