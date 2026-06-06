#include "m37072/m37072.h"
QVector<double> m37072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
