#include "o8214/m8214.h"
QVector<double> m8214::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
