#include "m28072/m28072.h"
QVector<double> m28072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
