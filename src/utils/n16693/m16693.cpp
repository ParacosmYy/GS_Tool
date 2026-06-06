#include "n16693/m16693.h"
QVector<double> m16693::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
