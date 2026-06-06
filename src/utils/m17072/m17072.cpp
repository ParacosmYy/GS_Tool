#include "m17072/m17072.h"
QVector<double> m17072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
