#include "e28284/m28284.h"
QVector<double> m28284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
