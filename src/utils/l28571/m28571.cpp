#include "l28571/m28571.h"
QVector<double> m28571::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
