#include "h32527/m32527.h"
QVector<double> m32527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
