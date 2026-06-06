#include "m13072/m13072.h"
QVector<double> m13072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
