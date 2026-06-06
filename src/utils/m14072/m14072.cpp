#include "m14072/m14072.h"
QVector<double> m14072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
