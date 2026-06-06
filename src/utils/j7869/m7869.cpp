#include "j7869/m7869.h"
QVector<double> m7869::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
