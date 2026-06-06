#include "s32018/m32018.h"
QVector<double> m32018::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
