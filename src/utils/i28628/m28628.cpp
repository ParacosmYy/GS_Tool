#include "i28628/m28628.h"
QVector<double> m28628::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
