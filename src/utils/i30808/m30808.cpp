#include "i30808/m30808.h"
QVector<double> m30808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
