#include "s15018/m15018.h"
QVector<double> m15018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
