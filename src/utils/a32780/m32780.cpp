#include "a32780/m32780.h"
QVector<double> m32780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
