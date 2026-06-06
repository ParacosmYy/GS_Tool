#include "d25303/m25303.h"
QVector<double> m25303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
