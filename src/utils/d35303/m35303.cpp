#include "d35303/m35303.h"
QVector<double> m35303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
