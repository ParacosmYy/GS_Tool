#include "s17018/m17018.h"
QVector<double> m17018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
