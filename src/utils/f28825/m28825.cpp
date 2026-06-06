#include "f28825/m28825.h"
QVector<double> m28825::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
