#include "d16303/m16303.h"
QVector<double> m16303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
