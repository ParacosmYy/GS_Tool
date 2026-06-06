#include "d15303/m15303.h"
QVector<double> m15303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
