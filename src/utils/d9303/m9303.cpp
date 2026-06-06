#include "d9303/m9303.h"
QVector<double> m9303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
