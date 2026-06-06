#include "l28911/m28911.h"
QVector<double> m28911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
