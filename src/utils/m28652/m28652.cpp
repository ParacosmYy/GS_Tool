#include "m28652/m28652.h"
QVector<double> m28652::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
