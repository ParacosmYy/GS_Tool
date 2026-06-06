#include "s7938/m7938.h"
QVector<double> m7938::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
