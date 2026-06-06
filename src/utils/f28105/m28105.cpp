#include "f28105/m28105.h"
QVector<double> m28105::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
