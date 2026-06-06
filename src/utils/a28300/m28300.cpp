#include "a28300/m28300.h"
QVector<double> m28300::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
