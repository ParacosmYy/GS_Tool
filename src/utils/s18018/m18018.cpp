#include "s18018/m18018.h"
QVector<double> m18018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
