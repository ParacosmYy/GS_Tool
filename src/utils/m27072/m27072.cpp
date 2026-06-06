#include "m27072/m27072.h"
QVector<double> m27072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
