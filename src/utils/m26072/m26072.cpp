#include "m26072/m26072.h"
QVector<double> m26072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
