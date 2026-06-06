#include "b18661/m18661.h"
QVector<double> m18661::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
