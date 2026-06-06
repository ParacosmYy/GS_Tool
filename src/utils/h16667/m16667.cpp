#include "h16667/m16667.h"
QVector<double> m16667::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
