#include "m28332/m28332.h"
QVector<double> m28332::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
