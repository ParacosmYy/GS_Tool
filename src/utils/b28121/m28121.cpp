#include "b28121/m28121.h"
QVector<double> m28121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
