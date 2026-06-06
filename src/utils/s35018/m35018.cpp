#include "s35018/m35018.h"
QVector<double> m35018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
