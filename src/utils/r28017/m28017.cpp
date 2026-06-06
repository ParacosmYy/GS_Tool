#include "r28017/m28017.h"
QVector<double> m28017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
