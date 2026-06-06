#include "m15072/m15072.h"
QVector<double> m15072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
