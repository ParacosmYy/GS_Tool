#include "p28115/m28115.h"
QVector<double> m28115::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
