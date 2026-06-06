#include "g27306/m27306.h"
QVector<double> m27306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
