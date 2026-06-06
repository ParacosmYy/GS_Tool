#include "s36018/m36018.h"
QVector<double> m36018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
