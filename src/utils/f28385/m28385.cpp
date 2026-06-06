#include "f28385/m28385.h"
QVector<double> m28385::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
