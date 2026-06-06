#include "p18215/m18215.h"
QVector<double> m18215::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
