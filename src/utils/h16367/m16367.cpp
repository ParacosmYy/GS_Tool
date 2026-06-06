#include "h16367/m16367.h"
QVector<double> m16367::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
