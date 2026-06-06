#include "d17583/m17583.h"
QVector<double> m17583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
