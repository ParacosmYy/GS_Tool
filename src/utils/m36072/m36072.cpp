#include "m36072/m36072.h"
QVector<double> m36072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
