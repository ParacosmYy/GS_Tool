#include "m28432/m28432.h"
QVector<double> m28432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
