#include "m28812/m28812.h"
QVector<double> m28812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
