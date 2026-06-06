#include "h35527/m35527.h"
QVector<double> m35527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
