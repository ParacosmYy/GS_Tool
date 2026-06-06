#include "m8072/m8072.h"
QVector<double> m8072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
