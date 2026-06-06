#include "d30303/m30303.h"
QVector<double> m30303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
