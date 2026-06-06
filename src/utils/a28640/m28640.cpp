#include "a28640/m28640.h"
QVector<double> m28640::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
