#include "f18205/m18205.h"
QVector<double> m18205::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
