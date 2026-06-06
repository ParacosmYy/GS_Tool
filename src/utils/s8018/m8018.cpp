#include "s8018/m8018.h"
QVector<double> m8018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
