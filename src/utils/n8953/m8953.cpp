#include "n8953/m8953.h"
QVector<double> m8953::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
