#include "d21303/m21303.h"
QVector<double> m21303::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
