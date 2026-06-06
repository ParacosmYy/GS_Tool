#include "d17303/m17303.h"
QVector<double> m17303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
