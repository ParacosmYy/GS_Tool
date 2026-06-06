#include "h18527/m18527.h"
QVector<double> m18527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
