#include "a28000/m28000.h"
QVector<double> m28000::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
